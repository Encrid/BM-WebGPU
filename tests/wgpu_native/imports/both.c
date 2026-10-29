/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
int other(void);
int runtime(void);

[[gnu::dllexport]] int both(void)
{
#ifdef SWAPPED
    int first = other();

    return (first + runtime());
#else
    int first = runtime();

    return (first + other());
#endif
}
