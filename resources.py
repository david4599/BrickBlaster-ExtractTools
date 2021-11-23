# This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

from pkg_resources import parse_version
import kaitaistruct
from kaitaistruct import KaitaiStruct, KaitaiStream, BytesIO


if parse_version(kaitaistruct.__version__) < parse_version('0.9'):
    raise Exception("Incompatible Kaitai Struct Python API: 0.9 or later is required, but you have %s" % (kaitaistruct.__version__))

class Resources(KaitaiStruct):
    def __init__(self, _io, _parent=None, _root=None):
        self._io = _io
        self._parent = _parent
        self._root = _root if _root else self
        self._read()

    def _read(self):
        self.header = Resources.Header(self._io, self, self._root)
        self.files = [None] * (self.header.num_files)
        for i in range(self.header.num_files):
            self.files[i] = Resources.File(self._io, self, self._root)


    class Header(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            self.magic = self._io.read_bytes(3)
            if not self.magic == b"\x45\x4F\x53":
                raise kaitaistruct.ValidationNotEqualError(b"\x45\x4F\x53", self.magic, self._io, u"/types/header/seq/0")
            self.unknown1 = self._io.read_u1()
            self.num_files = self._io.read_u1()
            self.unknown2 = self._io.read_bytes(15)


    class File(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            self.name = (self._io.read_bytes(12)).decode(u"UTF-8")
            self.size = self._io.read_u4le()
            self.offset = self._io.read_u4le()



