__all__ = ["ChildNotFoundError"]


class ChildNotFoundError(RuntimeError):
    def __str__(self):
        return repr(self)
