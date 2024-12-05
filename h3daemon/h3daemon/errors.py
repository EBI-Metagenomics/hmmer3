__all__ = ["ChildNotFoundError", "DaemonAlreadyRunningError", "PIDNotFoundError"]


class ChildNotFoundError(RuntimeError):
    def __str__(self):
        return repr(self)


class DaemonAlreadyRunningError(RuntimeError):
    def __str__(self):
        return repr(self)


class PIDNotFoundError(RuntimeError):
    def __str__(self):
        return repr(self)
