import subprocess

import optuna


def objective(trial):
    x = trial.suggest_float("x", 0.5, 5.0)
    y = trial.suggest_float("y", 0.5, 5.0)
    proc = subprocess.Popen(["./a.exe", str(x), str(y)], stdout=subprocess.PIPE)
    for line in proc.stdout:
        score = float(line.decode("utf-8"))
        break
    return score


study_name = "example-study"
study = optuna.create_study(study_name=study_name, storage="sqlite:///./optuna_study.db", load_if_exists=True)
study.optimize(objective, n_trials=100)
