import sys

# Args are <binary file> <header file> <array name>
with open(sys.argv[2],'wb') as result_file:
  result_file.write(b'#pragma once\n')
  result_file.write(b'namespace aka {\n')
  result_file.write(b'namespace font {\n')
  result_file.write(b'constexpr unsigned char %s[] = {\n\t' % sys.argv[3].encode('utf-8'))
  x=0
  for b in open(sys.argv[1], 'rb').read():
    result_file.write(b'0x%02X,' % b)
    x = x + 1
    if x > 30:
        x = 0
        result_file.write(b'\n\t')
  result_file.write(b'\n};')
  result_file.write(b'\n}; // namespace font')
  result_file.write(b'\n}; // namespace aka')