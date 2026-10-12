#include "Student.hpp"

#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>

static bool IsDate(const std::string& date)
{
    if (date.size() != 10 || date[4] != '-' || date[7] != '-')
    {
        return false;
    }
    for (int i = 0; i < 10; ++i)
    {
        if (i != 4 && i != 7 && !std::isdigit(static_cast<unsigned char>(date[i])))
        {
            return false;
        }
    }

    int year = std::stoi(date.substr(0, 4));
    int month = std::stoi(date.substr(5, 2));
    int day = std::stoi(date.substr(8, 2));
    if (year == 0 || month < 1 || month > 12)
    {
        return false;
    }

    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0))
    {
        days[1] = 29;
    }
    return day >= 1 && day <= days[month - 1];
}

Student::Student(int id, const std::string& firstName, const std::string& middleName,
                 const std::string& lastName, const std::string& birthDate)
    : id(id), firstName(firstName), middleName(middleName), lastName(lastName), birthDate(birthDate)
{
    if (id <= 0 || firstName.empty() || lastName.empty() || !IsDate(birthDate))
    {
        throw std::invalid_argument("Student: positive ID, name and date YYYY-MM-DD required");
    }
}

int Student::GetID() const { return this->id; }
const std::string& Student::GetFirstName() const { return this->firstName; }
const std::string& Student::GetMiddleName() const { return this->middleName; }
const std::string& Student::GetLastName() const { return this->lastName; }
const std::string& Student::GetBirthDate() const { return this->birthDate; }

bool CompareStudents(const Student& left, const Student& right)
{
    return left.GetID() < right.GetID();
}

std::string StudentToString(const Student& student)
{
    std::ostringstream output;
    output << student.GetID() << ' ' << std::quoted(student.GetFirstName()) << ' '
           << std::quoted(student.GetMiddleName()) << ' ' << std::quoted(student.GetLastName())
           << ' ' << std::quoted(student.GetBirthDate());
    return output.str();
}

Student StudentFromString(const std::string& text)
{
    std::istringstream input(text);
    int id;
    std::string firstName, middleName, lastName, birthDate;
    if (!(input >> id))
    {
        throw std::invalid_argument("StudentFromString: expected ID and four quoted fields");
    }

    std::string* fields[] = {&firstName, &middleName, &lastName, &birthDate};
    for (std::string* field : fields)
    {
        input >> std::ws;
        if (input.peek() != '"' || !(input >> std::quoted(*field)))
        {
            throw std::invalid_argument("StudentFromString: expected four quoted fields");
        }
    }

    input >> std::ws;
    if (!input.eof())
    {
        throw std::invalid_argument("StudentFromString: trailing data");
    }
    return Student(id, firstName, middleName, lastName, birthDate);
}
