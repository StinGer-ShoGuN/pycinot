# coding: utf-8

from ._cinot import __doc__
from ._cinot import inot


__all__ = ["inot"]


IN_ACCESS = 0x00000001                         #  File was accessed.
IN_MODIFY = 0x00000002                         #  File was modified.
IN_ATTRIB = 0x00000004                         #  Metadata changed.
IN_CLOSE_WRITE = 0x00000008	                   #  Writtable file was closed.
IN_CLOSE_NOWRITE = 0x00000010                  #  Unwrittable file closed.
IN_CLOSE = (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE) #  Close.
IN_OPEN = 0x00000020                           #  File was opened.
IN_MOVED_FROM = 0x00000040                     #  File was moved from X.
IN_MOVED_TO = 0x00000080                       #  File was moved to Y.
IN_MOVE = (IN_MOVED_FROM | IN_MOVED_TO)        #  Moves.
IN_CREATE = 0x00000100                         #  Subfile was created.
IN_DELETE = 0x00000200                         #  Subfile was deleted.
IN_DELETE_SELF = 0x00000400                    #  Self was deleted.
IN_MOVE_SELF = 0x00000800                      #  Self was moved.

