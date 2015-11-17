from json import load
import os.path
from os.path import join

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
    'uint': 'unsigned int',
    'date': 'std::chrono::system_clock::time_point'
}

TYPE_HDR_MAP = {
    'str': 'string',
    'ID': 'string',
    'date': 'chrono'
}

COMMON_TYPES = set((
    'std::string',
    'bool',
    'double',
    'int',
    'unsigned int'
))

RJ_GETTER = {
    'std::string': 'GetString',
    'bool': 'GetBool',
    'double': 'GetDouble',
    'int': 'GetInt',
    'unsigned int': 'GetUint'
}

STD_HEADERS = ('string', 'deque', 'list', 'functional', 'memory', 'bitset',
               'chrono')
RJ_HEADER = 'rapidjson/document'
RJ_ITR = 'rapidjson::Value::ConstValueIterator'

def includeHeaders(writer, headers):
    for header in headers:
        if isStdHeader(header):
            writer.writeln('#include <{0}>', header)
        else:
            writer.writeln('#include "{0}.h"', header)

def isStdHeader(header):
    return header in STD_HEADERS

def getType(typename):
    if typename in TYPE_MAP:
        return TYPE_MAP[typename]
    if typename[-2:] == '[]':
        typename = 'std::deque<{0}>'.format(getType(typename[:-2]))
    return typename

def getTypeHeader(typename, internal = False):
    postfix = '_internal' if internal else ''
    if typename in TYPE_HDR_MAP:
        return (TYPE_HDR_MAP[typename],)
    elif typename in TYPE_MAP:
        return ()
    if typename[-2:] == '[]':
        return getTypeHeader(typename[:-2], internal) + ('deque', )
    return (typename + postfix, )

def isListType(typename):
    return typename[-2:] == '[]'

def isCommonType(typename):
    return typename in COMMON_TYPES

def getListContentType(typename):
    return typename[:-2]

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

    body.write('void {0}::{1}::operator() (const callbackType &callback)', '::'.join(namespaces), method_name)
    header.writeln('using callbackType = std::function<void(std::shared_ptr<Account> account, const {0} &ret)>;', getType(value['ret']))
    header.writeln('void operator() (const callbackType &retCallback);')
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

    header.writeln('void callback(std::shared_ptr<Account> account, int code, const std::string &ret, const callbackType &callback);')

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
    body.writeln('std::placeholders::_3,')
    body.writeln('callback)')
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
    body.writeln('const std::string &ret,')
    body.writeln('const callbackType &callback)')
    body.scope_out()
    body.writeln('{{')
    body.scope_in()
    ret = value['ret']
    if ret == 'str':
        body.writeln('callback(account, ret);')
    else:
        body.writeln('rapidjson::Document dom;')
        body.writeln('dom.Parse(ret.c_str());')

        src_name = 'dom'
        target_name = 'results'
        body.writeln('{0} {1};', getType(ret), target_name);
        is_ret_list = isListType(ret)
        if is_ret_list:
            body.writeln('{0}.resize({1}.Size());', target_name, src_name)
            body.writeln('for (int i = 0, end = {0}.Size(); i < end; ++ i)',
                         src_name)
            body.writeln('{{')
            body.scope_in()
            src_name = '{0}[i]'.format(src_name)
            target_name = '{0}.at(i)'.format(target_name)
            ret = ret[:-2]

        body.writeln('Parse{0}({1}, {2});', ret.title(), target_name, src_name)
        if is_ret_list:
            body.scope_out()
            body.writeln('}}')
        body.writeln('callback(account, results);')
    body.scope_out()
    body.writeln('}}')
    header.writeln('}};')

def gen_type(args):
    root = args.root
    inc_dir = lambda x: join(root, 'include', 'gen', x)
    src_dir = lambda x: join(root, 'src', 'gen', x)
    with open(inc_dir(args.out + '.h'), 'w') as out_hdr_file, \
         open(src_dir(args.out + '.cpp'), 'w') as out_body_file:
        out_hdr = Writer(out_hdr_file)
        out_hdr.writeln(PREFIX_COMMENT)
        out_hdr.writeln('#pragma once')
        out_body = Writer(out_body_file)
        out_body.writeln(PREFIX_COMMENT)
        out_body.writeln('// This File is dummy for building')

        for define_path in args.defines:
            name = os.path.splitext(os.path.basename(define_path))[0]
            with open(define_path, 'r') as define_json:
                define = load(define_json)
                out_hdr.writeln('#include "{0}.h"', name)
                out_body.writeln('#include "{0}.cpp"', name)

            with open(inc_dir(name + '.h'), 'w') as def_hdr_file, \
                 open(src_dir(name + '_internal.h'), 'w') as def_hdr_int_file, \
                 open(src_dir(name + '.cpp'), 'w') as def_body_file:
                def_body = Writer(def_body_file)
                def_body.writeln(PREFIX_COMMENT)
                includeHeaders(def_body, (name + '_internal',
                                          RJ_HEADER, 'parseUtil'))
                def_body.writeln('')

                def_hdr = Writer(def_hdr_file)

                def_hdr_int = Writer(def_hdr_int_file)
                def_hdr_common = MultiWriter(def_hdr, def_hdr_int)
                def_hdr_common.writeln(PREFIX_COMMENT)

                def_hdr_common.writeln('#pragma once')
                headers = set()
                headers_internal = set(('gen/' + name, RJ_HEADER))
                for field in define.values():
                    headers.update(getTypeHeader(field['type']))
                    headers_internal.update(getTypeHeader(field['type'], True))
                includeHeaders(def_hdr, headers)
                includeHeaders(def_hdr_int, headers_internal)
                def_hdr_common.writeln('')

                for ns in args.namespace:
                    def_hdr_common.writeln('namespace {0}', ns)
                    def_hdr_common.writeln('{{')
                    def_hdr_common.scope_in()

                def_hdr_int.writeln('void Parse{0}({1} &ret,' + \
                                    'const rapidjson::Value &doc);',
                                    name.title(), name)

                def_hdr.writeln('class {0}', name)
                def_hdr.writeln('{{')
                def_hdr.writeln('public:')
                def_hdr.scope_in()

                def_body.writeln('void {2}::Parse{0}({1} &ret,' + \
                                 'const rapidjson::Value &doc)',
                                 name.title(), name, '::'.join(args.namespace))
                def_body.writeln('{{')
                def_body.scope_in()

                def writeParseField(fld_type, fld_name, fld_name_org, doc):
                    def_body.writeln('if ({0}.HasMember("{1}"))',
                                     doc, fld_name_org)
                    def_body.writeln('{{')
                    def_body.scope_in()
                    if isListType(fld_type):
                        val_name = fld_name_org + '_val'
                        def_body.writeln('const rapidjson::Value &{2} =' + \
                                         '{1}["{0}"];', fld_name_org, doc,
                                         val_name)
                        def_body.writeln('for (' + RJ_ITR + \
                                         ' {1}_itr = {0}.Begin(),' + \
                                         ' {1}_end = {0}.End();' + \
                                         ' {1}_itr != {1}_end;' + \
                                         '++{1}_itr)',
                                         val_name, fld_name_org)
                        def_body.writeln('{{')
                        def_body.scope_in()
                        writeParseField('', '', val_name)
                        def_body.scope_out()
                        def_body.writeln('}}')
                    elif isCommonType(fld_type):
                        def_body.writeln('{0} = {1}["{2}"].{3}();',
                                         fld_name, doc,
                                         fld_name_org, RJ_GETTER[fld_type])
                    else:
                        postfix = ''
                        if fld_type == TYPE_MAP['date']:
                            fld_type = 'time'
                            postfix = '.GetString()'
                        def_body.writeln('Parse{0}({1}, {2}["{3}"]{4});',
                                         fld_type.title(), fld_name,
                                         doc, fld_name_org, postfix)
                    def_body.scope_out()
                    def_body.writeln('}}')

                for field in define.items():
                    fld_name, fld_info = field[0], field[1]
                    fld_name_org = fld_name
                    if 'alias' in fld_info:
                        fld_name = fld_info['alias']
                    fld_type = getType(fld_info['type'])

                    def_hdr.writeln('{0} {1};', fld_type, fld_name)

                    writeParseField(fld_type, 'ret.' + fld_name, fld_name_org, 'doc')

                def_body.scope_out()
                def_body.writeln('}}')

                def_hdr.scope_out()
                def_hdr.writeln('}};')

                for ns in args.namespace:
                    def_hdr_common.scope_out()
                    def_hdr_common.writeln('}}')

def gen_api(args):
    inc_dir = lambda x: join(args.root, 'include', 'gen', x)
    src_dir = lambda x: join(args.root, 'src', 'gen', x)
    with open(inc_dir(args.out + '.h'), 'w') as out_header_file, \
         open(src_dir(args.out + '.cpp'), 'w') as out_body_file:

        out_header = Writer(out_header_file)
        out_header.writeln(PREFIX_COMMENT)
        out_header.writeln('#pragma once')
        out_header.writeln('')

        out_body = Writer(out_body_file)
        defines = {}
        for define in args.defines:
            with open(define, 'r') as define_json:
                new_def = load(define_json)
                defines.update(new_def)

        out_body.writeln(PREFIX_COMMENT)
        headers = set(('gen/' + args.out, 'request', RJ_HEADER))
        headers_pub = set(('api', 'string', 'functional', 'bitset'))
        for define in defines.values():
            req_hdr = getTypeHeader(define['ret'], True)
            headers.update(req_hdr)
            headers_pub.update(hdr for hdr in req_hdr
                               if not hdr.endswith('_internal'))

        includeHeaders(out_body, headers)
        includeHeaders(out_header, headers_pub)
        out_body.writeln('')
        out_header.writeln('')

        namespaces = args.namespace

        for ns in namespaces:
            out_header.writeln('namespace {0}', ns)
            out_header.writeln('{{')
            out_header.scope_in()

        for header in headers:
            if not header.endswith('_internal'):
                continue
            out_header.writeln('class {0};',
                               header.replace('_internal', ''))

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

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Generate api')
    parser.add_argument('defines', metavar='DEF', type=str, nargs='+',
                   help='API Definitions')
    parser.add_argument('-n', dest='namespace', type=str,
                        help='namespace')
    parser.add_argument('-o', dest='out', type=str,
                        help='output name', required=True)
    parser.add_argument('-d', dest='root', type=str,
                        help='output root')
    parser.add_argument('-t', dest='type', action='store_true')
    args = parser.parse_args()

    if not args.root:
        args.root = os.path.dirname(os.path.abspath(__file__))
    if not args.namespace:
        args.namespace = []
    else:
        args.namespace = args.namespace.split('::')

    try:
        os.mkdir(os.path.join(args.root, 'src', 'gen'))
        os.mkdir(os.path.join(args.root, 'include', 'gen'))
    except Exception:
        pass

    if args.type:
        gen_type(args)
    else:
        gen_api(args)
