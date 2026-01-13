import subprocess
if __name__ == "__main__":
    for i in range(4):
        target = pow(2, pow(3,i))
        res = subprocess.run(["./cmp", str(i)], capture_output=True).stdout.decode().split()[-1]
        print(f"For input {i} res: {res} target: {target}")
