# primitive data types

# OOP
class hustler(object):
    tag = "hustler"
    def __init__(self, name, gender, age, job):
        self.name = name
        self.gender = gender
        self.age = age
        self.job = job
        self.tag = tag

    def get_name(self):
        return self.name

    def get_age(self):
        if self.gender == "female":
            return 18
        else:
            return self.age

    def get_gender(self):
        pass

def main():
    rkname = "rock"
    rkgender = "male"
    rkage = 30
    rkjob = "BSP"
    rock = hustler(rkname.title(), rkgender, rkage, rkjob)
    print("Now!!")
    print(f"{rock.get_name()} is {rock.get_age()}")


if __name__ == "__mian__":
    main()
