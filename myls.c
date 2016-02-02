/*
 * Author: Kyle Bendickson
 * UCI ID: 18118767
 * HW #7: myls
 */

// Outputs with terminating slash, but should not.

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <langinfo.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

#define MAX_PATH_SIZE 256
#define MAX_SUB_DIRECTORIES 1024
#define MAX_DIR_CONTENT 1024

#define currentyear 2015

typedef int (*compfn)(const void*, const void*);

struct file_info_t {

	char path_to_file[MAX_PATH_SIZE];
	char permissions[12];
	char name[MAX_PATH_SIZE];
	char owner[MAX_PATH_SIZE];
	char group[MAX_PATH_SIZE];
	char datestring[MAX_PATH_SIZE];
	int num_links;
	int size;
	int is_dir;
};

int lstat(const char *path, struct stat *buf); // Forward declaration needed to suppress compiler warning.
char* get_permissions(mode_t st_mode);
int list_dir(char* path);
long long int calculate_total_blocks(char* path);
void build_path_str(char* ret_str, char* base_path, char* str_to_add);
void print_file(struct file_info_t file);
int cstr_comparator(const void* a, const void* b);
struct file_info_t get_file_info(char* parent_path, char* file_name);
int path_terminated_by_slash(const char*);
void strip_slash_from_path(char* stripped_path, const char *path);
void remove_char_from_string(char* input, char to_remove);


int main(int argc, char** argv) {

	/* Obtain the starting directory from args or default to cwd */
	char* path;
	char temp_path[MAX_PATH_SIZE];
	struct stat st;

	/* Get the starting path */
	if (argc > 1)
		path = argv[1];
	else {
		path = temp_path;
		path = getcwd(temp_path, 100);
	}

	// if (!path_terminated_by_slash(path)) {
	// 	strcat(path, "/");
	// }

	list_dir(path);

  	return 0;
}



int list_dir(char* path) {

	DIR* dir_ptr = NULL;
	struct dirent *dp = NULL;

	char* dircontent_arr[MAX_DIR_CONTENT];
	char subdir_arr[MAX_DIR_CONTENT][MAX_PATH_SIZE];

	for (int i = 0; i < MAX_DIR_CONTENT; ++i) {
		dircontent_arr[i] = malloc(sizeof(char) * MAX_PATH_SIZE);
	}


	int num_subdir = 0;
	int num_paths = 0;

	if (!path_terminated_by_slash(path)) {
		strcat(path, "/");
	}

	char output_path[256];
	strip_slash_from_path(output_path, path);
	/* Output path and total blocks. */
	printf("%s:\n", output_path);
	printf("total %lld\n", calculate_total_blocks(path));


	dir_ptr = opendir(path);
	if ( dir_ptr == NULL) {
	 	printf("\nlist_dir ERROR: Could not open the directory %s specified by argv.\n", path);
	 	exit(-1);
	}

	/* Iterate through all of directory contents and add the relative name to directory content array. 
	 * Ignore directories starting with '.'
	 */
	while ( (dp = readdir(dir_ptr)) ) {

		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 || dp->d_name[0] == '.') {
			continue;
		}

		strcpy(dircontent_arr[num_paths], dp->d_name);
		++num_paths;
	}

	/* Sort the directory contents alphabetically using a case ignoring comparison and also ignoring all '.' characters. */
	qsort(dircontent_arr, num_paths, sizeof(char*), cstr_comparator);

	/* Get and print the path info for each file while simultaneously collecting all of the subdirectories. */
	for (int i = 0; i < num_paths; ++i) {

		struct file_info_t file_info = get_file_info(path, dircontent_arr[i]);
		print_file(file_info);

		if (file_info.is_dir) {
			strcpy(subdir_arr[num_subdir], file_info.path_to_file);
			++num_subdir;
		}
	}

	/* Outputs the contents of all subdirectories */
	for (int i = 0; i < num_subdir; ++i) {
		printf("\n");
		list_dir(subdir_arr[i]);
	}

	closedir(dir_ptr);
	return 0;
}


long long int calculate_total_blocks(char* path) {

	DIR* dir_ptr = NULL;
	struct dirent *dp = NULL;
	struct stat st;
	long long int total_blocks = 0;

	if ( (dir_ptr = opendir(path)) == NULL) {
	 	printf("Calculate Total Blocks ERROR: Could not open the directory %s specified by argv.\n", path);
	 	return -1;
	}

	while ( (dp = readdir(dir_ptr)) != NULL ) {

		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 || dp->d_name[0] == '.') {
			continue;
		}

		char path_to_file[256];
		build_path_str(path_to_file, path, dp->d_name);

		lstat(path_to_file, &st);

		/* Get the total number of blocks */
		total_blocks += st.st_blocks;

	}

	closedir(dir_ptr);
	return total_blocks / 2; /* Divide by two to match centOS output */

}

void build_path_str(char* ret_str, char* base_path, char* str_to_add) {

	strncpy(ret_str, base_path, MAX_PATH_SIZE);

	if (!path_terminated_by_slash(base_path)) {
		strcat(ret_str, "/");
	}
	strcat(ret_str, str_to_add);
}


int cstr_comparator(const void* a, const void* b) {

	const char* aa = *(const char**)a;
    const char* bb = *(const char**)b;

    char aa_no_dot[MAX_PATH_SIZE];
    char bb_no_dot[MAX_PATH_SIZE];
    strcpy(aa_no_dot, aa);
    strcpy(bb_no_dot, bb);
    remove_char_from_string(aa_no_dot, '.');
    remove_char_from_string(bb_no_dot, '.');
    // printf("Comparing %s to %s\n", aa_no_dot, bb_no_dot);
    return strcasecmp(aa_no_dot, bb_no_dot);
	// return strcasecmp(aa, bb);
}

char* get_permissions(mode_t st_mode) {

	static char local_buff[16];
    local_buff[0] = (S_ISDIR(st_mode))  ? 'd' : '-';
    local_buff[1] = (st_mode & S_IRUSR) ? 'r' : '-';
    local_buff[2] = (st_mode & S_IWUSR) ? 'w' : '-';
    local_buff[3] = (st_mode & S_IXUSR) ? 'x' : '-';
    local_buff[4] = (st_mode & S_IRGRP) ? 'r' : '-';
    local_buff[5] = (st_mode & S_IWGRP) ? 'w' : '-';
    local_buff[6] = (st_mode & S_IXGRP) ? 'x' : '-';
    local_buff[7] = (st_mode & S_IROTH) ? 'r' : '-';
    local_buff[8] = (st_mode & S_IWOTH) ? 'w' : '-';
    local_buff[9] = (st_mode & S_IXOTH) ? 'x' : '-';

    return local_buff;
}

struct file_info_t get_file_info(char* parent_path, char* file_name) {

	struct file_info_t ret_file;
	struct stat st;
	struct passwd *pwd = NULL;
	struct group *grp = NULL;
	struct tm *tm = NULL;
	char datestring[256];
	char path_to_file[256];
	char subdir[MAX_SUB_DIRECTORIES][256];
	int total_subdir = 0;

	/* Copy path to file */
	build_path_str(path_to_file, parent_path, file_name);
	strcpy(ret_file.path_to_file, path_to_file);

	/* Copy file name */
	strcpy(ret_file.name, file_name);

	/* Copy struct stat */
	lstat(path_to_file, &st);

	ret_file.is_dir = (S_ISDIR(st.st_mode) ? 1 : 0);
	strcpy(ret_file.permissions, get_permissions(st.st_mode));
	ret_file.num_links = st.st_nlink;

	/* Get out owner's name */
	if ((pwd = getpwuid(st.st_uid)) != NULL)
		strcpy(ret_file.owner, pwd->pw_name);
	else
		printf(ret_file.owner, st.st_uid);


	/* Get group name */
	if ((grp = getgrgid(st.st_gid)) != NULL) {
		strcpy(ret_file.group, grp->gr_name);
	}
	else {
		char error_message[256];
		strcpy(error_message, "error: function getgrgid returned NULL.");
		strcpy(ret_file.group, error_message);
	}

	/* Get size of the file */
	ret_file.size = st.st_size;

	/* Get date string for file */
	tm = localtime(&st.st_mtime);

	/*
	 * Uses MM DD YYYY for anything before current year, otherwise uses MM DD HH:MM (24-hour time).
	 * Uses abbreviated english day of week for DD (e.g. Mon, Wed).
	 */
	const char* datestring_fmt = ((tm->tm_year + 1900) < currentyear ? "%b %d %Y" : "%b %d %H:%M");
	strftime(datestring, sizeof(datestring), datestring_fmt, tm);
	strcpy(ret_file.datestring, datestring);

	return ret_file;
}

void print_file(struct file_info_t file) {

	/* Print out type, permissions, and number of links. */
	// printf("%s", file.permissions);
	// printf("%4d", file.num_links);
	// printf(" %-8.8s", file.owner);
	// printf(" %-8.8s", file.group);
	// printf("%6d", file.size);
	// printf(" %s %s", file.datestring, file.name);
	// printf("\n");

	printf("%s", file.permissions);
	printf(" %d", file.num_links);
	printf(" %s", file.owner);
	printf(" %s", file.group);
	printf(" %d", file.size);
	printf(" %s %s", file.datestring, file.name);
	printf("\n");
}

int path_terminated_by_slash(const char *str)
{
    return (str[strlen(str) - 1] == '/') ? 1 : 0;
}

void strip_slash_from_path(char* stripped_path, const char *path) {
	strcpy(stripped_path, path);
	if (path_terminated_by_slash(path)) {
		stripped_path[strlen(stripped_path) - 1] = '\0';
	}
}

void remove_char_from_string(char* input, char to_remove) {


	char* src;
	char* dest;
	src = dest = input;
	while (*src != '\0') {
		if (*src != to_remove) {
			*dest = *src;
			dest++;
		}
		src++;
	}
	*dest = '\0';
}
