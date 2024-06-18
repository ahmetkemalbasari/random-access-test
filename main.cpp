#include <iostream>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <sys/stat.h>

//FILE OPERATIONS
FILE *dataFile = fopen("../data.dat", "w+");
FILE *indexFile = fopen("../index.dat", "w+");

int sizeOf(const std::string &path) {
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
int currentOverloadIndex;

int hashF(int id) {
    return (id % 100000);
}

Student searchStudent(int id) {
    int hashI = hashF(id);
    Node curNode{-1, -1, -1};
    Student student{0, 0, 0, false, ""};

    fseek(indexFile, hashI * sizeof(Node), SEEK_SET);
    fread(&curNode, sizeof(Node), 1, indexFile);

    //printf("searchNode: %d id, %d next node\n", curNode.id, curNode.nextNode);
    if (curNode.id == -1)
        return student;

    while (true) {
        if (curNode.id == id) {
            break;
        } else if (curNode.nextNode == -1)
            return student;

        printf("curNode: %d id, %d next node\n", curNode.id, curNode.nextNode);
        fseek(indexFile, (100000 + curNode.nextNode) * sizeof(Node), SEEK_SET);
        fread(&curNode, sizeof(Node), 1, indexFile);

        printf("curNode: %d id, %d next node\n", curNode.id, curNode.nextNode);
    }
    fseek(dataFile, curNode.dataIndex * sizeof(Student), SEEK_SET);
    fread(&student, sizeof(Student), 1, dataFile);

    return student;
}

bool iDExists(int id) {
    Student student = searchStudent(id);

    if (student.id != 0)
        return true;

    return false;
}

void addStudent(Student student) {
    int hashI = hashF(student.id);
    Node curIndex{-1, -1, -1};
    Node newIndex{student.id, -1, currentDataIndex};

    fseek(dataFile, sizeof(Student) * currentDataIndex, SEEK_SET);
    fwrite(&student, sizeof(Student), 1, dataFile);

    fseek(indexFile, hashI * sizeof(Node), SEEK_SET);
    fread(&curIndex, sizeof(Node), 1, indexFile);
    currentDataIndex++;

    printf("///// home: %d id, %d nextIndex \n", curIndex.id, curIndex.nextNode);

    if (curIndex.id == -1) {
        fseek(indexFile, hashI * sizeof(Node), SEEK_SET);
        fwrite(&newIndex, sizeof(Node), 1, indexFile);
        printf("newHome: %d id, %d nextIndex \n", newIndex.id, newIndex.nextNode);
        return;
    }

    newIndex.nextNode = curIndex.nextNode;
    curIndex.nextNode = currentOverloadIndex;

    fseek(indexFile, sizeof(Node) * (currentOverloadIndex + 100000), SEEK_SET);
    fwrite(&newIndex, sizeof(Node), 1, indexFile);

    fseek(indexFile, hashI * sizeof(Node), SEEK_SET);
    fwrite(&curIndex, sizeof(Node), 1, indexFile);

    printf("newIndex: %d id, %d nextIndex ///// home: %d id, %d nextIndex \n", newIndex.id, newIndex.nextNode,
           curIndex.id, curIndex.nextNode);
    currentOverloadIndex++;
    return;
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
    memcpy(&student.name, &splitted[1], 32);

    return student;
}


int main() {

    Node indexData{-1, -1, -1};
    for (int i = 0; i < 100000; i++) {
        fwrite(&indexData, sizeof(Node), 1, indexFile);
    }

    //currentDataIndex = sizeOf("../data.dat") / sizeof(Student);
    //currentOverloadIndex = (sizeOf("../index.dat") / sizeof(Node)) - 100000;
    currentDataIndex = 0;
    currentOverloadIndex = 0;


    int i = 0;
    for (std::string line: readLines("../students.txt")) {
        Student student = convertToStudent(line);
        printf("%d \n", i);
        if (!iDExists(student.id)) {
            addStudent(student);
        }
        if (++i == 1000000)
            break;
    }


    return 0;
}
/*
    IndexData indexData{-1, -1, -1};
    for (int i = 0; i < 100000; i++) {
        fwrite(&indexData, sizeof(IndexData), 1, indexFile);
    }

*/

/*
 #include <iostream>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <sys/stat.h>

//FILE OPERATIONS
FILE *dataFile = fopen("../data.dat", "rb+");
FILE *indexFile = fopen("../index.dat", "rb+");

int sizeOf(const std::string &path) {
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

struct IndexData {
    int id;
    int next;
    int index;
};


//DATA OPERATIONS
int currentData;
int currentIndex;

int hash(int id) {
    return id % 100000;
}

bool checkIndex(int id) {
    int home = hash(id);
    IndexData indexData{};

    fseek(indexFile, home * sizeof(IndexData), SEEK_SET);
    fread(&indexData, sizeof(IndexData), 1, indexFile);

    while (id != indexData.id){
        home = indexData.next;
        fseek(indexFile, (home + 100000) * sizeof(IndexData), SEEK_SET);
        fread(&indexData, sizeof(IndexData), 1, indexFile);

        if(indexData.next == -1)
            break;
    }

    return indexData.id != -1;
}

void saveStudent(Student student) {
    fseek(dataFile, 0, SEEK_SET);
    fwrite(&student, sizeof(Student), 1, dataFile);

    IndexData newIndex{student.id, -1, currentData};
    IndexData indexData{};

    int home = hash(student.id);

    fseek(indexFile, home * sizeof(IndexData), SEEK_SET);
    fread(&indexData, sizeof(IndexData), 1, indexFile);

    if (indexData.id == -1) {
        fseek(indexFile, home * sizeof(IndexData), SEEK_SET);
        fwrite(&newIndex, sizeof(IndexData), 1, indexFile);
        return;
    }

    newIndex.next = indexData.next;
    indexData.next = currentIndex++;

    fseek(indexFile, (currentIndex + 99999) * sizeof(IndexData), SEEK_SET);
    fwrite(&newIndex, sizeof(IndexData), 1, indexFile);

    fseek(indexFile, home * sizeof(IndexData), SEEK_SET);
    fwrite(&indexData, sizeof(IndexData), 1, indexFile);

}

Student searchStudent(int id) {
    int home = hash(id);
    IndexData indexData{};
    fseek(indexFile, home * sizeof(IndexData), SEEK_SET);
    fread(&indexData, sizeof(IndexData), 1, indexFile);
    while (id != indexData.id){
        home = indexData.next;
        fseek(indexFile, (home + 100000) * sizeof(IndexData), SEEK_SET);
        fread(&indexData, sizeof(IndexData), 1, indexFile);

        if(indexData.next == -1)
            break;
    }

    if (indexData.id == -1)
        return Student{};

    Student student{};

    fseek(dataFile, indexData.index * sizeof(Student), SEEK_SET);
    fread(&student, sizeof(Student), 1, dataFile);
    return student;
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
    memcpy(&student.name, &splitted[1], 32);

    return student;
}


int main() {
    currentData = sizeOf("../data.dat") / sizeof(Student);
    currentIndex = sizeOf("../index.dat") / sizeof(IndexData) - 100000;


    int i = 0;
    for (std::string line: readLines("../students.txt")) {
        Student student = convertToStudent(line);
        if (!checkIndex(student.id)) {
            saveStudent(student);
        }

        if (++i == 1000000)
            break;
    }


    Student s = searchStudent(782014280);
    printf("%d, %s", s.id, s.name);

    return 0;
}
/*
    IndexData indexData{-1, -1, -1};
    for (int i = 0; i < 100000; i++) {
        fwrite(&indexData, sizeof(IndexData), 1, indexFile);
    }

*/