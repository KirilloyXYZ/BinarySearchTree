#ifndef STUDENT_HPP
#define STUDENT_HPP

#include <string>

class Student
{
private:
    int id;
    std::string firstName;
    std::string middleName;
    std::string lastName;
    std::string birthDate;

public:
    Student();
    Student(int id, const std::string& firstName, const std::string& middleName,
            const std::string& lastName, const std::string& birthDate);

    int GetID() const;
    const std::string& GetFirstName() const;
    const std::string& GetMiddleName() const;
    const std::string& GetLastName() const;
    const std::string& GetBirthDate() const;
};

bool CompareStudents(const Student& left, const Student& right);
std::string StudentToString(const Student& student);
Student StudentFromString(const std::string& text);

#endif
