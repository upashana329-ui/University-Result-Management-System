#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STUDENTS 100
#define MAX_SUBJECTS 6
#define PASSWORD "admin123"  // Admin password for result entry

typedef struct {
    char subject_code[10];
    char subject_name[30];
    int credits;
    float grade_points;  // 10 point scale
    char grade[3];
} Subject;

typedef struct {
    int roll_no;
    char name[50];
    char branch[30];
    int semester;
    Subject subjects[MAX_SUBJECTS];
    int num_subjects;
    float sgpa;
    float cgpa;  // For simplicity, CGPA = SGPA in this demo
} Student;

// Function prototypes
void admin_menu();
void student_menu();
void add_student();
void add_results();
void view_all_students();
void generate_report_card(int roll_no);
float calculate_sgpa(Student s);
char* calculate_grade(float gp);
void display_student_summary(Student s);
int authenticate();
void save_to_file();
void load_from_file();
int is_student_exists(int roll_no);
void update_cgpa();

// Global file for storage
const char *FILENAME = "student_records.dat";

int main() {
    int choice;
    
    load_from_file();  // Load existing records at startup
    
    while (1) {
        printf("\n========================================\n");
        printf("   UNIVERSITY RESULT MANAGEMENT SYSTEM   \n");
        printf("========================================\n");
        printf("1. Admin (Result Entry & Management)\n");
        printf("2. Student (View Results)\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch(choice) {
            case 1:
                if(authenticate()) {
                    admin_menu();
                } else {
                    printf("\nAccess Denied! Invalid Password.\n");
                }
                break;
            case 2:
                student_menu();
                break;
            case 3:
                printf("\nExiting System. Goodbye!\n");
                exit(0);
            default:
                printf("\nInvalid choice! Try again.\n");
        }
    }
    return 0;
}

// Authentication for admin
int authenticate() {
    char password[20];
    printf("\nEnter Admin Password: ");
    scanf("%s", password);
    return strcmp(password, PASSWORD) == 0;
}

// Admin Menu
void admin_menu() {
    int choice;
    while(1) {
        printf("\n========== ADMIN MENU ==========\n");
        printf("1. Add New Student\n");
        printf("2. Enter/Update Results\n");
        printf("3. View All Students Summary\n");
        printf("4. Generate Report Card\n");
        printf("5. Exit to Main Menu\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        
        switch(choice) {
            case 1: add_student(); break;
            case 2: add_results(); break;
            case 3: view_all_students(); break;
            case 4: 
                {
                    int roll;
                    printf("Enter Roll Number: ");
                    scanf("%d", &roll);
                    generate_report_card(roll);
                }
                break;
            case 5: return;
            default: printf("Invalid choice!\n");
        }
    }
}

// Student Menu
void student_menu() {
    int roll_no;
    printf("\nEnter your Roll Number: ");
    scanf("%d", &roll_no);
    generate_report_card(roll_no);
}

// Add new student
void add_student() {
    FILE *fp = fopen(FILENAME, "ab");
    if(!fp) {
        printf("Error opening file!\n");
        return;
    }
    
    Student s;
    printf("\n--- Add New Student ---\n");
    printf("Roll Number: ");
    scanf("%d", &s.roll_no);
    
    if(is_student_exists(s.roll_no)) {
        printf("Student with this Roll Number already exists!\n");
        fclose(fp);
        return;
    }
    
    printf("Student Name: ");
    scanf(" %[^\n]", s.name);
    printf("Branch: ");
    scanf(" %[^\n]", s.branch);
    printf("Semester: ");
    scanf("%d", &s.semester);
    
    s.num_subjects = 0;
    s.sgpa = 0;
    s.cgpa = 0;
    
    fwrite(&s, sizeof(Student), 1, fp);
    fclose(fp);
    printf("Student added successfully!\n");
}

// Add or update results for a student
void add_results() {
    int roll_no, i;
    printf("\nEnter Roll Number to enter results: ");
    scanf("%d", &roll_no);
    
    FILE *fp = fopen(FILENAME, "rb+");
    if(!fp) {
        printf("No records found!\n");
        return;
    }
    
    Student s;
    int found = 0;
    long pos;
    
    while(fread(&s, sizeof(Student), 1, fp)) {
        if(s.roll_no == roll_no) {
            found = 1;
            pos = ftell(fp) - sizeof(Student);
            break;
        }
    }
    
    if(!found) {
        printf("Student not found!\n");
        fclose(fp);
        return;
    }
    
    printf("\nEnter results for: %s (Roll: %d)\n", s.name, s.roll_no);
    printf("Number of subjects (max %d): ", MAX_SUBJECTS);
    scanf("%d", &s.num_subjects);
    
    if(s.num_subjects > MAX_SUBJECTS) s.num_subjects = MAX_SUBJECTS;
    
    for(i = 0; i < s.num_subjects; i++) {
        printf("\nSubject %d:\n", i+1);
        printf("  Subject Code: ");
        scanf("%s", s.subjects[i].subject_code);
        printf("  Subject Name: ");
        scanf(" %[^\n]", s.subjects[i].subject_name);
        printf("  Credits: ");
        scanf("%d", &s.subjects[i].credits);
        printf("  Grade Points (0-10): ");
        scanf("%f", &s.subjects[i].grade_points);
        
        if(s.subjects[i].grade_points >= 9.0) strcpy(s.subjects[i].grade, "A+");
        else if(s.subjects[i].grade_points >= 8.0) strcpy(s.subjects[i].grade, "A");
        else if(s.subjects[i].grade_points >= 7.0) strcpy(s.subjects[i].grade, "B+");
        else if(s.subjects[i].grade_points >= 6.0) strcpy(s.subjects[i].grade, "B");
        else if(s.subjects[i].grade_points >= 5.0) strcpy(s.subjects[i].grade, "C");
        else if(s.subjects[i].grade_points >= 4.0) strcpy(s.subjects[i].grade, "D");
        else strcpy(s.subjects[i].grade, "F");
        
        // Validate grade points
        if(s.subjects[i].grade_points < 0 || s.subjects[i].grade_points > 10) {
            printf("Invalid grade points! Setting to 0.\n");
            s.subjects[i].grade_points = 0;
            strcpy(s.subjects[i].grade, "F");
        }
    }
    
    // Calculate SGPA
    s.sgpa = calculate_sgpa(s);
    s.cgpa = s.sgpa;  // For simplicity, assuming CGPA = current SGPA
    
    // Update in file
    fseek(fp, pos, SEEK_SET);
    fwrite(&s, sizeof(Student), 1, fp);
    fclose(fp);
    
    printf("\nResults added successfully!\n");
    printf("SGPA: %.2f\n", s.sgpa);
}

// Calculate SGPA
float calculate_sgpa(Student s) {
    float total_points = 0;
    int total_credits = 0;
    
    for(int i = 0; i < s.num_subjects; i++) {
        total_points += s.subjects[i].grade_points * s.subjects[i].credits;
        total_credits += s.subjects[i].credits;
    }
    
    if(total_credits == 0) return 0;
    return total_points / total_credits;
}

// Generate detailed report card
void generate_report_card(int roll_no) {
    FILE *fp = fopen(FILENAME, "rb");
    if(!fp) {
        printf("No records found!\n");
        return;
    }
    
    Student s;
    int found = 0;
    
    while(fread(&s, sizeof(Student), 1, fp)) {
        if(s.roll_no == roll_no) {
            found = 1;
            break;
        }
    }
    fclose(fp);
    
    if(!found) {
        printf("Student with Roll Number %d not found!\n", roll_no);
        return;
    }
    
    printf("\n");
    printf("========================================================\n");
    printf("              UNIVERSITY REPORT CARD                     \n");
    printf("========================================================\n");
    printf("Roll Number: %d\n", s.roll_no);
    printf("Student Name: %s\n", s.name);
    printf("Branch: %s\n", s.branch);
    printf("Semester: %d\n", s.semester);
    printf("--------------------------------------------------------\n");
    printf("%-10s %-25s %-8s %-12s %s\n", "Code", "Subject", "Credits", "Grade Points", "Grade");
    printf("--------------------------------------------------------\n");
    
    for(int i = 0; i < s.num_subjects; i++) {
        printf("%-10s %-25s %-8d %-12.2f %s\n", 
               s.subjects[i].subject_code,
               s.subjects[i].subject_name,
               s.subjects[i].credits,
               s.subjects[i].grade_points,
               s.subjects[i].grade);
    }
    
    printf("--------------------------------------------------------\n");
    printf("SGPA: %.2f\n", s.sgpa);
    printf("CGPA: %.2f\n", s.cgpa);
    
    // Display result status
    int failed = 0;
    for(int i = 0; i < s.num_subjects; i++) {
        if(s.subjects[i].grade_points < 4.0) {
            failed = 1;
            break;
        }
    }
    
    if(failed) {
        printf("\nRESULT: FAILED (Improvement needed in one or more subjects)\n");
    } else {
        printf("\nRESULT: PASSED\n");
        if(s.sgpa >= 8.5) printf("Remarks: Outstanding Performance!\n");
        else if(s.sgpa >= 7.0) printf("Remarks: Good Performance\n");
        else if(s.sgpa >= 5.0) printf("Remarks: Satisfactory\n");
        else printf("Remarks: Needs Improvement\n");
    }
    printf("========================================================\n");
}

// View all students summary
void view_all_students() {
    FILE *fp = fopen(FILENAME, "rb");
    if(!fp) {
        printf("No records found!\n");
        return;
    }
    
    Student s;
    int count = 0;
    
    printf("\n%-10s %-25s %-15s %-8s %s\n", "Roll No", "Name", "Branch", "Semester", "SGPA");
    printf("----------------------------------------------------------------\n");
    
    while(fread(&s, sizeof(Student), 1, fp)) {
        printf("%-10d %-25s %-15s %-8d %.2f\n", 
               s.roll_no, s.name, s.branch, s.semester, s.sgpa);
        count++;
    }
    
    if(count == 0) {
        printf("No students found!\n");
    } else {
        printf("----------------------------------------------------------------\n");
        printf("Total Students: %d\n", count);
    }
    
    fclose(fp);
}

// Check if student exists
int is_student_exists(int roll_no) {
    FILE *fp = fopen(FILENAME, "rb");
    if(!fp) return 0;
    
    Student s;
    while(fread(&s, sizeof(Student), 1, fp)) {
        if(s.roll_no == roll_no) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

// Save data to file (already handled by individual operations)
void save_to_file() {
    // Data is saved immediately in add_student and add_results functions
    printf("Data saved successfully!\n");
}

// Load data from file at startup
void load_from_file() {
    FILE *fp = fopen(FILENAME, "rb");
    if(fp) {
        // Just check if file exists, no action needed
        fclose(fp);
        printf("Loaded existing records from file.\n");
    }
}

// Utility function to convert grade points to letter grade
char* calculate_grade(float gp) {
    if(gp >= 9.0) return "A+";
    else if(gp >= 8.0) return "A";
    else if(gp >= 7.0) return "B+";
    else if(gp >= 6.0) return "B";
    else if(gp >= 5.0) return "C";
    else if(gp >= 4.0) return "D";
    else return "F";
}