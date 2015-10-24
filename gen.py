from json import load

REQUEST_TYPE = {
    'POST': 'Quacks::Twit::Request::RequestType::POST',
    'GET': 'Quacks::Twit::Request::RequestType::GET'
}

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

    def scopde_out(self):
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

def print_method(header, body, method_info):
    namespaces = method_info[0].split('/')
    method_name = namespaces[-1]
    namespaces = namespaces[0:-1]
    value = method_info[1]
    both = MultiWriter(header, body)
    for ns in namespaces:
        both.writeln('namespace {0}', ns)
        both.writeln('{{')
        both.scope_in()

    header.writeln('///')
    header.writeln('/// @summary {0}', value['description'])
    header.writeln('struct {0} : APIObject', method_name)
    header.writeln('\{')
    header.scope_in()
    body.write('void {0}::operator() (', '::'.join(namespaces))
    header.writeln('void operator() (')
    both.scope_in()
    both.scope_out()
    both.write(')')
    header.writeln(';')
    body.writeln('')
    body.writeln('\{')
    body.scope_in()
    #body
    body.writeln('}')
    body.scope_out()

    for ns in namespaces:
        header.write('}')
        header.scope_out()


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
        with open(os.path.join(args.root, 'src', args.out + '.cpp'), 'w') \
             as out_body_file:
            out_body = Writer(out_body_file)
            defines = {}
            for define in args.defines:
                with open(define, 'r') as define_json:
                    new_def = load(define_json)
                    defines.update(new_def)

            for define in defines.items():
                print_method(out_header, out_body, define)
