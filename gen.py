from json import load

PREFIX_COMMENT = '''//////////////////////////////
///                        ///
/// This code is GENERATED ///
///                        ///
//////////////////////////////
'''

REQUEST_TYPE = {
    'POST': 'Quacks::Twit::RequestType::POST',
    'GET': 'Quacks::Twit::RequestType::GET'
}
SEND_REQUEST = 'Quacks::Twit::Request::sendRequest'
ARGS_TYPE = 'Quacks::Twit::Request::RequestArgType'

TYPE_MAP = {
    'str': 'std::string',
    'ID': 'std::string',
    'bool': 'bool',
    'dbl': 'double',
    'int': 'int',
    'uint': 'unsigned int'
}

def toString(t, val):
    if t in ('str', 'ID'):
        return val
    return 'std::to_string({0})'.format(val)

class Writer:
    def __init__(self, f):
        self.f = f
        self.indent = 0

    def writeln(self, line, *args):
        self.f.write('{0}{1}\n'.format(self.indent * '  ', line.format(*args)))

    def write(self, line, *args):
        self.f.write(line.format(*args))

    def scope_in(self):
        self.indent += 1

    def scope_out(self):
        self.indent -= 1

class MultiWriter:
    def __init__(self, *args):
        self.f = args

    def writeln(self, line, *args):
        for f in self.f:
            f.writeln(line, *args)

    def write(self, line, *args):
        for f in self.f:
            f.write(line, *args)

    def scope_in(self):
        for f in self.f:
            f.scope_in()

    def scope_out(self):
        for f in self.f:
            f.scope_out()

def print_method(header, body, prefix_ns, method_info):
    namespaces = method_info[0].split('/')
    method_name = namespaces[-1]
    namespaces = prefix_ns + namespaces[0:-1]
    value = method_info[1]
    both = MultiWriter(header, body)

    if 'description' in value:
        header.writeln('///')
        header.writeln('/// @summary {0}', value['description'])
    header.writeln('struct {0} : APIObject', method_name)
    header.writeln('{{')
    header.scope_in()

    body.write('void {0}::{1}::operator() (', '::'.join(namespaces), method_name)
    header.writeln('void operator() (')
    both.scope_in()
    both.scope_out()
    both.write(')')
    header.write(';')
    both.write('\n')

    params = sorted(value['params'].items())

    #params - header
    header.scope_out()
    header.writeln('private:')
    header.scope_in()
    for param in params:
        name, param = param
        if param.get('required', False):
            header.writeln('// @required')
        header.writeln('{0} _{1};', TYPE_MAP[param['type']], name)
    if len(params) > 0:
        header.writeln('std::bitset<{0}> _setted;', len(params))

    header.writeln('void callback(std::shared_ptr<Account> account, int code, const std::string &ret);')

    header.scope_out()
    header.writeln('public:')
    header.scope_in()
    param_index = 0
    for param in params:
        name, param = param
        header.writeln('{0}& {1}(const {2} &val)', method_name, name, TYPE_MAP[param['type']])
        header.writeln('{{')
        header.scope_in()
        header.writeln('_{0} = val;', name)
        header.writeln('_setted[{0}] = true;', param_index)
        header.writeln('return *this;')
        header.scope_out()
        header.writeln('}}')
        param_index += 1

    body.writeln('{{')
    body.scope_in()
    #body
    body.writeln('{0} args;', ARGS_TYPE)
    body.write('\n')
    def fill_param(param, index):
        body.writeln('if (_setted[{0}])', index)
        body.writeln('{{')
        body.scope_in()
        name = param[1].get('name', None) or param[0]
        body.writeln('args.push_back(')
        body.scope_in()
        body.writeln('std::make_pair(')
        body.scope_in()
        body.writeln('"{0}",', name)
        body.writeln('{0}));', toString(param[1]['type'], '_' + param[0]))
        body.scope_out()
        body.scope_out()
        body.scope_out()
        body.writeln('}}')
        if param[1].get('required', False):
            body.writeln('else')
            body.writeln('{{')
            body.scope_in()
            body.writeln('throw std::exception();')
            body.scope_out()
            body.writeln('}}')
        body.write('\n')

    param_index = 0
    for param in params:
        fill_param(param, param_index)
        param_index += 1

    body.writeln(SEND_REQUEST)
    body.writeln('(')
    body.scope_in()
    body.writeln('{0},', REQUEST_TYPE[value['request']])
    body.writeln('account,')
    body.writeln('"{0}",', value['url'])
    body.writeln('args,')
    body.writeln('std::bind(')
    body.scope_in()
    body.writeln('&{0}::callback,', method_name)
    body.writeln('this,')
    body.writeln('std::placeholders::_1,')
    body.writeln('std::placeholders::_2,')
    body.writeln('std::placeholders::_3)')
    body.scope_out()
    body.scope_out()
    body.writeln(');')

    both.scope_out()
    body.writeln('}}')

    body.write('\n')
    body.writeln('void {0}::{1}::callback(', '::'.join(namespaces), method_name)
    body.scope_in()
    body.writeln('std::shared_ptr<Account> account,')
    body.writeln('int code,')
    body.writeln('const std::string &ret)')
    body.scope_out()
    body.writeln('{{')
    body.scope_in()
    body.scope_out()
    body.writeln('}}')
    header.writeln('}};')


if __name__ == '__main__':
    import argparse, os.path
    parser = argparse.ArgumentParser(description='Generate api')
    parser.add_argument('defines', metavar='DEF', type=str, nargs='+',
                   help='API Definitions')
    parser.add_argument('-n', dest='namespace', type=str,
                        help='namespace')
    parser.add_argument('-o', dest='out', type=str,
                        help='output name', required=True)
    parser.add_argument('-d', dest='root', type=str,
                        help='output root')
    args = parser.parse_args()

    if not args.root:
        args.root = os.path.dirname(os.path.abspath(__file__))
    if not args.namespace:
        args.namespace = []
    else:
        args.namespace = args.namespace.split('::')

    with open(os.path.join(args.root, 'include', args.out + '.h'), 'w') \
         as out_header_file:
        out_header = Writer(out_header_file)
        out_header.writeln(PREFIX_COMMENT)
        out_header.writeln('#pragma once')
        out_header.writeln('#include "api.h"')
        for std_header in ('string', 'functional', 'bitset'):
            out_header.writeln('#include <{0}>', std_header)
        out_header.write('\n')
        with open(os.path.join(args.root, 'src', args.out + '.cpp'), 'w') \
             as out_body_file:
            out_body = Writer(out_body_file)
            out_body.writeln(PREFIX_COMMENT)
            out_body.writeln('#include "{0}.h"', args.out)
            out_body.writeln('#include "request.h"')
            out_body.write('\n')
            defines = {}
            for define in args.defines:
                with open(define, 'r') as define_json:
                    new_def = load(define_json)
                    defines.update(new_def)

            namespaces = args.namespace

            for ns in namespaces:
                out_header.writeln('namespace {0}', ns)
                out_header.writeln('{{')
                out_header.scope_in()

            prev_ns = None

            for define in sorted(defines.items()):
                cur_ns = define[0].split('/', 1)[0]
                if cur_ns != prev_ns:
                    if prev_ns:
                        out_header.scope_out()
                        out_header.writeln('}}')
                    out_header.writeln('namespace {0}', cur_ns)
                    out_header.writeln('{{')
                    out_header.scope_in()
                    prev_ns = cur_ns
                print_method(out_header, out_body, args.namespace, define)

            if prev_ns:
                namespaces.append(prev_ns)

            for ns in namespaces:
                out_header.scope_out()
                out_header.writeln('}}')
