/**
 * @file tsds.cpp
 * @brief Public-facing module of the project.
 * Doesn't really do anything other than:
 * - Tie other modules into a large one.
 * - Define document groups.
 * - And some other document-related stuff.
 */

#ifdef TSDS_MODULE
module;
export module tsds;
export import tsds.pool_alloc;
#endif // TSDS_MODULE
