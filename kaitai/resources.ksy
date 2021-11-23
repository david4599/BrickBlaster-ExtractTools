meta:
  id: resources
  endian: le
seq:
  - id: header
    type: header
  - id: files
    type: file
    repeat: expr
    repeat-expr: header.num_files
types:
  header:
    seq:
      - id: magic
        contents: 'EOS'
      - id: unknown1
        type: u1
      - id: num_files
        type: u1
      - id: unknown2
        size: 15
  file:
    seq:
      - id: name
        type: str
        size: 12 # 8.3 short filename
        encoding: UTF-8
      - id: size
        type: u4
      - id: offset
        type: u4