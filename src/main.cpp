#include <iostream>
#include <cstring>
#include <filesystem>
#include <vector>
#include <sstream>
#include <fstream>
#include <sys/stat.h>

#define NODE_SIZE (int) sizeof(Node)
#define STUDENT_SIZE (int) sizeof(Student)

#define INDEX_FILE_COUNT 100000
#define INDEX_FILE_SIZE (STUDENT_SIZE * INDEX_FILE_COUNT)


//FILE OPERATIONS
FILE *dataFile;
FILE *indexFile;

int fileSize(const std::string &path) {
    struct stat file_stat{};
    stat(path.data(), &file_stat);
    return static_cast<int>(file_stat.st_size);
}

std::vector<std::string> readLines(const std::string &path) {
    std::vector<std::string> lines;
    std::ifstream file(path);
    std::string str;
    while (std::getline(file, str))
        lines.push_back(str);

    return lines;
}


//STRUCTS
struct Student {
    int id;
    char age;
    char grade;
    bool gender;
    char name[32];
};

struct Node {
    int id;
    int nextNode;
    int dataIndex;
};


// DATA OPS
int currentDataIndex;
int currentOverflowIndex;

int hash(int id) {
    return (id % INDEX_FILE_COUNT);
}

Student searchStudent(int id) {
    int homeAddress = hash(id);

    Node curNode{};
    fseek(indexFile, homeAddress * NODE_SIZE, SEEK_SET);
    fread(&curNode, NODE_SIZE, 1, indexFile);

    if (curNode.id == 0 || curNode.id == -1)
        return {};

    while (true) {
        if (curNode.id == id)
            break;
        if (curNode.nextNode == -1)
            return {};

        fseek(indexFile, (curNode.nextNode + INDEX_FILE_COUNT) * NODE_SIZE, SEEK_SET);
        fread(&curNode, sizeof(Node), 1, indexFile);
    }

    Student student{};
    fseek(dataFile, STUDENT_SIZE * curNode.dataIndex, SEEK_SET);
    fread(&student, STUDENT_SIZE, 1, dataFile);

    return student;
}

bool existsStudent(int id){
    return searchStudent(id).id != 0;
}

void addStudent(Student student) {
    int homeAddress = hash(student.id);
    Node newIndex{student.id, -1, currentDataIndex};

    fseek(dataFile, STUDENT_SIZE * currentDataIndex++, SEEK_SET);
    fwrite(&student, STUDENT_SIZE, 1, dataFile);

    Node curIndex{};
    fseek(indexFile, homeAddress * NODE_SIZE, SEEK_SET);
    fread(&curIndex, sizeof(Node), 1, indexFile);

    if (curIndex.id == 0) {
        fseek(indexFile, homeAddress * NODE_SIZE, SEEK_SET);
        fwrite(&newIndex, sizeof(Node), 1, indexFile);
        return;
    }

    newIndex.nextNode = curIndex.nextNode;
    curIndex.nextNode = currentOverflowIndex++;

    fseek(indexFile, (currentOverflowIndex + INDEX_FILE_COUNT - 1) * NODE_SIZE, SEEK_SET);
    fwrite(&newIndex, sizeof(Node), 1, indexFile);

    fseek(indexFile, homeAddress * NODE_SIZE, SEEK_SET);
    fwrite(&curIndex, sizeof(Node), 1, indexFile);
}

Student convertToStudent(const std::string &string) {
    Student student{0, 0, 0, false, ""};

    std::vector<std::string> splitted;
    std::string token;
    std::istringstream tokenStream(string);
    while (std::getline(tokenStream, token, '|')) {
        splitted.push_back(token);
    }

    student.id = stol(splitted[0]);
    student.gender = splitted[2] == "Kadın";
    student.age = stoi(splitted[3]);
    student.grade = stoi(splitted[4]);
    memcpy(&student.name, splitted[1].c_str(), 32);

    return student;
}


int main() {
    if (!std::filesystem::exists("../data.dat"))
        std::ofstream index("../data.dat");
    if (!std::filesystem::exists("../index.dat"))
        std::ofstream index("../index.dat");
    if (std::filesystem::file_size("../index.dat") < INDEX_FILE_SIZE)
        std::filesystem::resize_file("../index.dat", INDEX_FILE_SIZE);

    dataFile = fopen("../data.dat", "rb+");
    indexFile = fopen("../index.dat", "rb+");

    currentDataIndex = fileSize("../data.dat") / STUDENT_SIZE;
    currentOverflowIndex = fileSize("../index.dat") / NODE_SIZE - 100000;

    for (std::string line: readLines("../students.txt")) {
        Student student = convertToStudent(line);

        if(!existsStudent(student.id))
            addStudent(student);
    }

    srand((unsigned)time(0));
    int n;

    std::vector<Student> students;
    for(int j = 0; j < 500000; j++){
        Student student;
        n = (rand()%10000000);

        fseek(dataFile, STUDENT_SIZE*n, SEEK_SET);
        fread(&student, STUDENT_SIZE, 1, dataFile);

        students.push_back(student);
    }

    auto start = std::chrono::high_resolution_clock::now();

    for(Student s : students)
        searchStudent(s.id);

    auto finish = std::chrono::high_resolution_clock::now();
    std::cout << "\nAverage seek time:" <<std::chrono::duration_cast<std::chrono::nanoseconds>(finish-start).count() / 1000000.0 << "ms\n";


    Student student = searchStudent(550362344);
    printf("%d %s %d %d\n", student.id, student.name, student.age, student.grade);

    return 0;
}
