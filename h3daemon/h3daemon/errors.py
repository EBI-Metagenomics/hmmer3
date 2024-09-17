__all__ = ["ParentNotAliveError", "ChildNotFoundError", "CouldNotPossessError"]


class ParentNotAliveError(RuntimeError): ...


class ChildNotFoundError(RuntimeError): ...


class CouldNotPossessError(RuntimeError): ...
