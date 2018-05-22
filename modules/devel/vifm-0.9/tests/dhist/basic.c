#include <stic.h>

#include "../../src/dhist.h"

SETUP()
{
	assert_success(dhist_set_size(10U));
}

TEARDOWN()
{
	assert_success(dhist_set_size(0U));
}

TEST(saving_fails_if_history_size_is_zero)
{
	assert_success(dhist_set_size(0U));
	assert_int_equal(dhist_save(-1, "/", "etc", 0), -1);
}

TEST(first_history_item_has_zero_position)
{
	const int dhpos = dhist_save(-1, "/", "etc", 0);
	assert_int_equal(0, dhpos);
}

TEST(last_history_item_does_not_repeat)
{
	int dhpos = -1;

	dhpos = dhist_save(dhpos, "/", "etc", 0);
	assert_int_equal(0, dhpos);

	dhpos = dhist_save(dhpos, "/", "etc", 4);
	assert_int_equal(0, dhpos);
}

TEST(edge_history_item_is_updated)
{
	int dhpos = -1;

	dhpos = dhist_save(dhpos, "/etc", "alsa", 4);
	assert_success(dhist_set_size(1U));
	(void)dhist_save(dhpos, "/etc", "rc.d", 8);
}

TEST(shrinking_works)
{
	int dhpos = -1;

	assert_success(dhist_set_size(2U));

	dhpos = dhist_save(dhpos, "/", "etc", 0);
	dhpos = dhist_save(dhpos, "/etc", "rc.d", 4);

	assert_success(dhist_set_size(1U));

	dhpos = dhist_save(dhpos, "/etc", "alsa", 4);
	assert_int_equal(0, dhpos);
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
