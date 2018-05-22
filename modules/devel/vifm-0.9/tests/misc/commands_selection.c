#include <stic.h>

#include <unistd.h> /* chdir() symlink() */

#include <stdio.h> /* remove() */
#include <string.h> /* strcpy() strdup() */

#include "../../src/cfg/config.h"
#include "../../src/ui/ui.h"
#include "../../src/utils/dynarray.h"
#include "../../src/utils/fs.h"
#include "../../src/utils/str.h"
#include "../../src/cmd_core.h"

#include "utils.h"

static void add_some_files_to_view(FileView *view);

static char cwd[PATH_MAX];

SETUP_ONCE()
{
	assert_non_null(get_cwd(cwd, sizeof(cwd)));
}

SETUP()
{
	view_setup(&lwin);
	view_setup(&rwin);

	curr_view = &lwin;
	other_view = &rwin;

#ifndef _WIN32
	replace_string(&cfg.shell, "/bin/sh");
#else
	replace_string(&cfg.shell, "cmd");
#endif

	stats_update_shell_type(cfg.shell);

	init_commands();
}

TEARDOWN()
{
	stats_update_shell_type("/bin/sh");
	update_string(&cfg.shell, NULL);

	view_teardown(&lwin);
	view_teardown(&rwin);
}

TEST(select_fails_for_wrong_pattern)
{
	assert_failure(exec_commands("select /**/", &lwin, CIT_COMMAND));
}

TEST(select_fails_for_pattern_and_range)
{
	assert_failure(exec_commands("1,$select *.c", &lwin, CIT_COMMAND));
}

TEST(select_selects_matching_files)
{
	add_some_files_to_view(&lwin);

	assert_success(exec_commands("select *.c", &lwin, CIT_COMMAND));

	assert_int_equal(2, lwin.selected_files);
	assert_true(lwin.dir_entry[0].selected);
	assert_true(lwin.dir_entry[2].selected);
}

TEST(select_appends_matching_files_to_selection)
{
	add_some_files_to_view(&lwin);
	lwin.dir_entry[0].selected = 1;
	lwin.dir_entry[1].selected = 1;
	lwin.selected_files = 2;

	assert_success(exec_commands("select *.c", &lwin, CIT_COMMAND));

	assert_int_equal(3, lwin.selected_files);
	assert_true(lwin.dir_entry[0].selected);
	assert_true(lwin.dir_entry[1].selected);
	assert_true(lwin.dir_entry[2].selected);
}

TEST(select_bang_unselects_nonmatching_files)
{
	add_some_files_to_view(&lwin);
	lwin.dir_entry[0].selected = 1;
	lwin.dir_entry[1].selected = 1;
	lwin.selected_files = 2;

	assert_success(exec_commands("select! *.c", &lwin, CIT_COMMAND));

	assert_int_equal(2, lwin.selected_files);
	assert_true(lwin.dir_entry[0].selected);
	assert_false(lwin.dir_entry[1].selected);
	assert_true(lwin.dir_entry[2].selected);
}

TEST(select_noargs_selects_current_file)
{
	add_some_files_to_view(&lwin);
	lwin.dir_entry[0].selected = 1;
	lwin.dir_entry[1].selected = 1;
	lwin.selected_files = 2;
	lwin.list_pos = 2;

	assert_success(exec_commands("select", &lwin, CIT_COMMAND));
	assert_int_equal(3, lwin.selected_files);
	assert_true(lwin.dir_entry[0].selected);
	assert_true(lwin.dir_entry[1].selected);
	assert_true(lwin.dir_entry[2].selected);

	assert_success(exec_commands("select", &lwin, CIT_COMMAND));
	assert_int_equal(3, lwin.selected_files);
	assert_true(lwin.dir_entry[0].selected);
	assert_true(lwin.dir_entry[1].selected);
	assert_true(lwin.dir_entry[2].selected);

	assert_success(exec_commands("select!", &lwin, CIT_COMMAND));
	assert_int_equal(1, lwin.selected_files);
	assert_false(lwin.dir_entry[0].selected);
	assert_false(lwin.dir_entry[1].selected);
	assert_true(lwin.dir_entry[2].selected);
}

TEST(select_can_select_range)
{
	add_some_files_to_view(&lwin);
	lwin.dir_entry[0].selected = 1;
	lwin.selected_files = 1;

	assert_success(exec_commands("2,$select", &lwin, CIT_COMMAND));
	assert_int_equal(3, lwin.selected_files);
	assert_true(lwin.dir_entry[0].selected);
	assert_true(lwin.dir_entry[1].selected);
	assert_true(lwin.dir_entry[2].selected);

	assert_success(exec_commands("2,$select!", &lwin, CIT_COMMAND));
	assert_int_equal(2, lwin.selected_files);
	assert_false(lwin.dir_entry[0].selected);
	assert_true(lwin.dir_entry[1].selected);
	assert_true(lwin.dir_entry[2].selected);
}

TEST(unselect_fails_for_wrong_pattern)
{
	assert_failure(exec_commands("unselect /**/", &lwin, CIT_COMMAND));
}

TEST(unselect_fails_for_pattern_and_range)
{
	assert_failure(exec_commands("1,$unselect *.c", &lwin, CIT_COMMAND));
}

TEST(unselect_unselects_matching_files)
{
	add_some_files_to_view(&lwin);
	lwin.dir_entry[0].selected = 1;
	lwin.dir_entry[1].selected = 1;
	lwin.selected_files = 2;

	assert_success(exec_commands("unselect *.c", &lwin, CIT_COMMAND));

	assert_int_equal(1, lwin.selected_files);
	assert_false(lwin.dir_entry[0].selected);
	assert_true(lwin.dir_entry[1].selected);
	assert_false(lwin.dir_entry[2].selected);
}

TEST(unselect_noargs_unselects_current_file)
{
	add_some_files_to_view(&lwin);
	lwin.dir_entry[0].selected = 1;
	lwin.dir_entry[1].selected = 1;
	lwin.selected_files = 2;

	lwin.list_pos = 2;
	assert_success(exec_commands("unselect", &lwin, CIT_COMMAND));
	assert_int_equal(2, lwin.selected_files);
	assert_true(lwin.dir_entry[0].selected);
	assert_true(lwin.dir_entry[1].selected);
	assert_false(lwin.dir_entry[2].selected);

	lwin.list_pos = 1;
	assert_success(exec_commands("unselect", &lwin, CIT_COMMAND));
	assert_int_equal(1, lwin.selected_files);
	assert_true(lwin.dir_entry[0].selected);
	assert_false(lwin.dir_entry[1].selected);
	assert_false(lwin.dir_entry[2].selected);
}

TEST(unselect_can_unselect_range)
{
	add_some_files_to_view(&lwin);
	lwin.dir_entry[0].selected = 1;
	lwin.dir_entry[1].selected = 1;
	lwin.dir_entry[2].selected = 1;
	lwin.selected_files = 3;

	assert_success(exec_commands("2,$unselect", &lwin, CIT_COMMAND));
	assert_int_equal(1, lwin.selected_files);
	assert_true(lwin.dir_entry[0].selected);
	assert_false(lwin.dir_entry[1].selected);
	assert_false(lwin.dir_entry[2].selected);
}

TEST(select_and_unselect_and_pattern_ambiguity)
{
	add_some_files_to_view(&lwin);

	assert_success(exec_commands("select !{*.c}", &lwin, CIT_COMMAND));
	assert_success(exec_commands("unselect !/\\.c/", &lwin, CIT_COMMAND));
	assert_int_equal(1, lwin.selected_files);
	assert_false(lwin.dir_entry[0].selected);
	assert_true(lwin.dir_entry[1].selected);
	assert_false(lwin.dir_entry[2].selected);
}

TEST(select_and_unselect_use_last_pattern)
{
	add_some_files_to_view(&lwin);

	cfg_resize_histories(5);

	cfg_save_search_history(".*\\.C");
	assert_success(exec_commands("select! //I", &lwin, CIT_COMMAND));
	assert_int_equal(0, lwin.selected_files);
	assert_false(lwin.dir_entry[0].selected);
	assert_false(lwin.dir_entry[1].selected);
	assert_false(lwin.dir_entry[2].selected);

	cfg_save_search_history(".*\\.c$");
	assert_success(exec_commands("select //", &lwin, CIT_COMMAND));
	assert_int_equal(2, lwin.selected_files);
	assert_true(lwin.dir_entry[0].selected);
	assert_false(lwin.dir_entry[1].selected);
	assert_true(lwin.dir_entry[2].selected);

	cfg_save_search_history("a.c");
	assert_success(exec_commands("unselect ////", &lwin, CIT_COMMAND));
	assert_int_equal(1, lwin.selected_files);
	assert_false(lwin.dir_entry[0].selected);
	assert_false(lwin.dir_entry[1].selected);
	assert_true(lwin.dir_entry[2].selected);

	cfg_resize_histories(0);
}

TEST(select_and_unselect_accept_external_command)
{
	strcpy(lwin.curr_dir, cwd);
	assert_success(chdir(cwd));

	add_some_files_to_view(&lwin);

	assert_success(exec_commands("select !echo a.c", &lwin, CIT_COMMAND));
	assert_int_equal(1, lwin.selected_files);
	assert_true(lwin.dir_entry[0].selected);
	assert_false(lwin.dir_entry[1].selected);
	assert_false(lwin.dir_entry[2].selected);

	assert_success(exec_commands("select!!echo c.c", &lwin, CIT_COMMAND));
	assert_int_equal(1, lwin.selected_files);
	assert_false(lwin.dir_entry[0].selected);
	assert_false(lwin.dir_entry[1].selected);
	assert_true(lwin.dir_entry[2].selected);

	assert_success(exec_commands("unselect !echo c.c", &lwin, CIT_COMMAND));
	assert_int_equal(0, lwin.selected_files);
	assert_false(lwin.dir_entry[0].selected);
	assert_false(lwin.dir_entry[1].selected);
	assert_false(lwin.dir_entry[2].selected);
}

TEST(select_expands_macros_in_external_command)
{
	strcpy(lwin.curr_dir, cwd);
	assert_success(chdir(cwd));

	add_some_files_to_view(&lwin);

	assert_success(exec_commands("select !echo %c", &lwin, CIT_COMMAND));
	assert_int_equal(1, lwin.selected_files);
	assert_true(lwin.dir_entry[0].selected);
	assert_false(lwin.dir_entry[1].selected);
	assert_false(lwin.dir_entry[2].selected);
}

TEST(select_and_unselect_consider_trailing_slash)
{
	strcpy(lwin.curr_dir, cwd);
	assert_success(chdir(cwd));

	lwin.list_rows = 4;
	lwin.list_pos = 0;
	lwin.dir_entry = dynarray_cextend(NULL,
			lwin.list_rows*sizeof(*lwin.dir_entry));
	lwin.dir_entry[0].name = strdup("a");
	lwin.dir_entry[0].origin = &lwin.curr_dir[0];
	lwin.dir_entry[0].type = FT_REG;
	lwin.dir_entry[1].name = strdup("a");
	lwin.dir_entry[1].origin = &lwin.curr_dir[0];
	lwin.dir_entry[1].type = FT_DIR;
	lwin.dir_entry[2].name = strdup("b");
	lwin.dir_entry[2].origin = &lwin.curr_dir[0];
	lwin.dir_entry[2].type = FT_REG;
	lwin.dir_entry[3].name = strdup("b");
	lwin.dir_entry[3].origin = &lwin.curr_dir[0];
	lwin.dir_entry[3].type = FT_DIR;
	lwin.selected_files = 0;

	/* Select only directories. */
	assert_success(exec_commands("select */", &lwin, CIT_COMMAND));
	assert_int_equal(2, lwin.selected_files);
	assert_false(lwin.dir_entry[0].selected);
	assert_true(lwin.dir_entry[1].selected);
	assert_false(lwin.dir_entry[2].selected);
	assert_true(lwin.dir_entry[3].selected);

	/* Select both file and directory. */
	assert_success(exec_commands("select! a", &lwin, CIT_COMMAND));
	assert_int_equal(2, lwin.selected_files);
	assert_true(lwin.dir_entry[0].selected);
	assert_true(lwin.dir_entry[1].selected);
	assert_false(lwin.dir_entry[2].selected);
	assert_false(lwin.dir_entry[3].selected);

	/* Select only files inside given directory. */
	assert_success(exec_commands("select! {{*/a/**}}", &lwin, CIT_COMMAND));
	assert_int_equal(0, lwin.selected_files);
	assert_false(lwin.dir_entry[0].selected);
	assert_false(lwin.dir_entry[1].selected);
	assert_false(lwin.dir_entry[2].selected);
	assert_false(lwin.dir_entry[3].selected);

	/* Select directories and files. */
	assert_success(exec_commands("select! {{*/a}}", &lwin, CIT_COMMAND));
	assert_int_equal(2, lwin.selected_files);
	assert_true(lwin.dir_entry[0].selected);
	assert_true(lwin.dir_entry[1].selected);
	assert_false(lwin.dir_entry[2].selected);
	assert_false(lwin.dir_entry[3].selected);

	/* Select only directories. */
	assert_success(exec_commands("select! {{*/a/}}", &lwin, CIT_COMMAND));
	assert_int_equal(1, lwin.selected_files);
	assert_false(lwin.dir_entry[0].selected);
	assert_true(lwin.dir_entry[1].selected);
	assert_false(lwin.dir_entry[2].selected);
	assert_false(lwin.dir_entry[3].selected);
}

TEST(symlinks_are_not_resolved_in_cwd, IF(not_windows))
{
#ifndef _WIN32
	assert_success(symlink(TEST_DATA_PATH "/existing-files",
				SANDBOX_PATH "/link"));
#endif

	lwin.list_rows = 1;
	lwin.list_pos = 0;
	lwin.dir_entry = dynarray_cextend(NULL,
			lwin.list_rows*sizeof(*lwin.dir_entry));
	lwin.dir_entry[0].name = strdup("a");
	lwin.dir_entry[0].origin = &lwin.curr_dir[0];
	lwin.dir_entry[0].type = FT_REG;
	lwin.selected_files = 0;

	/* Select only directories. */
	assert_success(exec_commands("select !echo a", &lwin, CIT_COMMAND));
	assert_int_equal(1, lwin.selected_files);
	assert_true(lwin.dir_entry[0].selected);

	assert_success(remove(SANDBOX_PATH "/link"));
}

static void
add_some_files_to_view(FileView *view)
{
	view->list_rows = 3;
	view->list_pos = 0;
	view->dir_entry = dynarray_cextend(NULL,
			view->list_rows*sizeof(*view->dir_entry));
	view->dir_entry[0].name = strdup("a.c");
	view->dir_entry[0].origin = &view->curr_dir[0];
	view->dir_entry[1].name = strdup("b.cc");
	view->dir_entry[1].origin = &view->curr_dir[0];
	view->dir_entry[2].name = strdup("c.c");
	view->dir_entry[2].origin = &view->curr_dir[0];
	view->selected_files = 0;
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
