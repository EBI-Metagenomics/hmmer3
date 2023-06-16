import os

__all__ = ["PathLike"]

PathLike = str | bytes | os.PathLike
