__all__ = ["ParentNotAliveError", "ChildNotFoundError", "CouldNotPossessError"]


class ParentNotAliveError(RuntimeError):
    def __str__(self):
        return repr(self)


class ChildNotFoundError(RuntimeError):
    def __str__(self):
        return repr(self)


class CouldNotPossessError(RuntimeError):
    def __str__(self):
        return repr(self)
