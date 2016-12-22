#ifndef __SHELL_COMMAND__H__
#define __SHELL_COMMAND__H__

#include <stdio.h>
#include <stdlib.h>

#include "fs.h"

typedef int (shell_cmd_f) (int argc, char* argv[]);

int cmd_cd(int argc, char* argv[]);
int cmd_pwd(int argc, char* argv[]);

int cmd_ls(int argc, char* argv[]);

int cmd_mkdir(int argc, char* argv[]);
int cmd_rmdir(int argc, char* argv[]);

int cmd_cp(int argc, char* argv[]);

int cmd_mv(int argc, char* argv[]);

int cmd_rm(int argc, char* argv[]);

int cmd_tree(int argc, char* argv[]);

int cmd_ln(int argc, char* argv[]);

int cmd_stat(int argc, char* argv[]);

int cmd_cat(int argc, char* argv[]);
int cmd_createfile(int argc, char* argv[]);

int cmd_df(int argc, char* argv[]);



#endif /* __SHELL_COMMAND__H__ */
