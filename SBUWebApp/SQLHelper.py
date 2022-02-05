from typing import Optional
from datetime import datetime


class WhereClause:
    query: str

    def __init__(self):
        self.query = ""

    def Query(self):
        return self.query

    def add_and(self, exp: str):
        if (self.query == ""):
            self.query = f"WHERE {exp}"
        else:
            self.query = f"{self.query} AND {exp}"

    def add_op_date(self, field: str, op: str, value: Optional[datetime]):
        if (value):
            self.add_and(f"{field} {op} date(\'{value}\')")

    def add_op_int(self, field: str, op: str, value: Optional[int]):
        if (value):
            self.add_and(f"{field} {op} {value}")
