/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
int other(void);

[[gnu::dllexport]] int user(void)
{
    return (other());
}
