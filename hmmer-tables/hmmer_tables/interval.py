from __future__ import annotations

from dataclasses import dataclass

__all__ = ["PyInterval", "RInterval"]


@dataclass(frozen=True)
class PyInterval:
    """
    Python interval.

    A Python interval is an 0-start, half-open interval. It means that:
    - The elements of a sequence have the coordinates `0, 1, 2, ...`.
    - An interval `PyInterval(start, stop)` is defined by the coordinates
      `start, start+1, ..., stop-2, stop-1`.

    Attributes
    ----------
    start
        Start of interval. Valid values are `0, 1, ..., stop`.
    stop
        End of interval. Valid values are `start, start+1, ...`.
    """

    start: int
    stop: int

    def __post_init__(self):
        if self.start > self.stop:
            raise ValueError(f"Invalid PyInterval({self.start}, {self.stop}).")

    def to_rinterval(self) -> RInterval:
        return RInterval(self.start + 1, self.stop)

    def to_slice(self) -> slice:
        return slice(self.start, self.stop)

    def offset(self, offset: int) -> PyInterval:
        return PyInterval(self.start + offset, self.stop + offset)


@dataclass(frozen=True)
class RInterval:
    """
    R interval.

    An R interval is an 1-start, fully-closed. It means that:
    - The elements of a sequence have the coordinates `1, 2, 3, ...`.
    - An interval `RInterval(start, stop)` is defined by the coordinates
      `start, start+1, ..., stop-1, stop`.
    """

    start: int
    stop: int

    def __post_init__(self):
        if self.start > self.stop:
            raise ValueError(f"Invalid RInterval({self.start}, {self.stop}).")

    def to_pyinterval(self) -> PyInterval:
        return PyInterval(self.start - 1, self.stop)

    def to_slice(self) -> slice:
        return self.to_pyinterval().to_slice()

    def offset(self, offset: int) -> RInterval:
        return RInterval(self.start + offset, self.stop + offset)
