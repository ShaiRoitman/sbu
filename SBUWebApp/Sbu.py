class SbuEnv:
    sbu_path: str
    repository_db: str
    files_db: str
    backup_db: str

class SbuApp:
    def __init__(self):
        pass

    def execute(self, cmdLine: str) -> {int, str}:
        pass