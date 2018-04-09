cmake_minimum_required(VERSION 3.2)
project(berkeleydb)

find_package(Threads)

if(WIN32)
set(DB_PLATFORM_HEADERS
	build_windows/clib_port.h
	build_windows/db.h
	build_windows/dbstl_common.h
	build_windows/db_config.h
	build_windows/db_cxx.h
	build_windows/db_int.h
)

set(DB_PLATFORM_SRC
os_windows/ce_ctime.c
os_windows/os_abs.c
os_windows/os_clock.c
os_windows/os_config.c
os_windows/os_cpu.c
os_windows/os_dir.c
os_windows/os_errno.c
os_windows/os_fid.c
os_windows/os_flock.c
os_windows/os_fsync.c
os_windows/os_getenv.c
os_windows/os_handle.c
os_windows/os_map.c
os_windows/os_mkdir.c
os_windows/os_open.c
os_windows/os_rename.c
os_windows/os_rw.c
os_windows/os_seek.c
os_windows/os_stat.c
os_windows/os_truncate.c
os_windows/os_unlink.c
os_windows/os_yield.c

mutex/mut_win32.c

repmgr/repmgr_windows.c

)
endif()

set(DB_SRC
crypto/aes_method.c
crypto/crypto.c
crypto/mersenne/mt19937db.c
crypto/rijndael/rijndael-alg-fst.c
crypto/rijndael/rijndael-api-fst.c

btree/btree_auto.c
#btree/btree_autop.c
btree/bt_compact.c
btree/bt_compare.c
btree/bt_compress.c
btree/bt_conv.c
btree/bt_curadj.c
btree/bt_cursor.c
btree/bt_delete.c
btree/bt_method.c
btree/bt_open.c
btree/bt_put.c
btree/bt_rec.c
btree/bt_reclaim.c
btree/bt_recno.c
btree/bt_rsearch.c
btree/bt_search.c
btree/bt_split.c
btree/bt_stat.c
btree/bt_upgrade.c
btree/bt_verify.c

db/crdel_auto.c
#db/crdel_autop.c
db/crdel_rec.c
db/db.c
db/db_am.c
db/db_auto.c
#db/db_autop.c
db/db_cam.c
db/db_cds.c
db/db_conv.c
db/db_dispatch.c
db/db_dup.c
db/db_iface.c
db/db_join.c
db/db_meta.c
db/db_method.c
db/db_open.c
db/db_overflow.c
db/db_ovfl_vrfy.c
db/db_pr.c
db/db_rec.c
db/db_reclaim.c
db/db_remove.c
db/db_rename.c
db/db_ret.c
db/db_setid.c
db/db_setlsn.c
db/db_sort_multiple.c
db/db_stati.c
db/db_truncate.c
db/db_upg.c
db/db_upg_opd.c
db/db_vrfy.c
db/db_vrfyutil.c
#db/db_vrfy_stub.c
db/partition.c

cxx/cxx_db.cpp
cxx/cxx_dbc.cpp
cxx/cxx_dbt.cpp
cxx/cxx_env.cpp
cxx/cxx_except.cpp
cxx/cxx_lock.cpp
cxx/cxx_logc.cpp
cxx/cxx_mpool.cpp
cxx/cxx_multi.cpp
cxx/cxx_seq.cpp
cxx/cxx_txn.cpp

dbreg/dbreg.c
dbreg/dbreg_auto.c
#dbreg/dbreg_autop.c
dbreg/dbreg_rec.c
dbreg/dbreg_stat.c
dbreg/dbreg_util.c

#common/crypto_stub.c
common/dbt.c
common/db_byteorder.c
common/db_compint.c
common/db_err.c
common/db_getlong.c
common/db_idspace.c
common/db_log2.c
common/db_shash.c
common/mkpath.c
common/openflags.c
common/os_method.c
common/util_arg.c
common/util_cache.c
common/util_log.c
common/util_sig.c
common/zerofill.c

env/env_alloc.c
env/env_config.c
env/env_failchk.c
env/env_file.c
env/env_globals.c
env/env_method.c
env/env_name.c
env/env_open.c
env/env_recover.c
env/env_region.c
env/env_register.c
env/env_sig.c
env/env_stat.c

fileops/fileops_auto.c
fileops/fop_basic.c
fileops/fop_rec.c
fileops/fop_util.c

hash/hash.c
hash/hash_auto.c
hash/hash_conv.c
hash/hash_dup.c
hash/hash_func.c
hash/hash_meta.c
hash/hash_method.c
hash/hash_open.c
hash/hash_page.c
hash/hash_rec.c
hash/hash_reclaim.c
hash/hash_stat.c
hash/hash_upgrade.c
hash/hash_verify.c

hmac/hmac.c
hmac/sha1.c

hsearch/hsearch.c

lock/lock.c
lock/lock_deadlock.c
lock/lock_failchk.c
lock/lock_id.c
lock/lock_list.c
lock/lock_method.c
lock/lock_region.c
lock/lock_stat.c
lock/lock_timer.c
lock/lock_util.c

log/log.c
log/log_archive.c
log/log_compare.c
log/log_debug.c
log/log_get.c
log/log_method.c
log/log_put.c
log/log_stat.c

mp/mp_alloc.c
mp/mp_bh.c
mp/mp_fget.c
mp/mp_fmethod.c
mp/mp_fopen.c
mp/mp_fput.c
mp/mp_fset.c
mp/mp_method.c
mp/mp_mvcc.c
mp/mp_region.c
mp/mp_register.c
mp/mp_resize.c
mp/mp_stat.c
mp/mp_sync.c
mp/mp_trickle.c

mutex/mut_alloc.c
mutex/mut_failchk.c
mutex/mut_method.c
mutex/mut_region.c
mutex/mut_stat.c

qam/qam.c
qam/qam_auto.c
qam/qam_conv.c
qam/qam_files.c
qam/qam_method.c
qam/qam_open.c
qam/qam_rec.c
qam/qam_stat.c
qam/qam_upgrade.c
qam/qam_verify.c

rep/rep_auto.c
rep/rep_backup.c
rep/rep_elect.c
rep/rep_lease.c
rep/rep_log.c
rep/rep_method.c
rep/rep_record.c
rep/rep_region.c
rep/rep_stat.c
rep/rep_util.c
rep/rep_verify.c

repmgr/repmgr_auto.c
repmgr/repmgr_elect.c
repmgr/repmgr_method.c
repmgr/repmgr_msg.c
repmgr/repmgr_net.c
repmgr/repmgr_queue.c
repmgr/repmgr_sel.c
repmgr/repmgr_stat.c
repmgr/repmgr_util.c

sequence/sequence.c
sequence/seq_stat.c

#clib/atoi.c
#clib/atol.c
#clib/getcwd.c
#clib/getopt.c
#clib/isalpha.c
#clib/isdigit.c
#clib/isprint.c
#clib/isspace.c
#clib/memcmp.c
#clib/memmove.c
#clib/printf.c
#clib/qsort.c
#clib/raise.c
#clib/rand.c
#clib/snprintf.c
#clib/strcasecmp.c
#clib/strcat.c
#clib/strchr.c
#clib/strdup.c
#clib/strerror.c
#clib/strncat.c
#clib/strncmp.c
#clib/strrchr.c
clib/strsep.c
#clib/strtol.c
#clib/strtoul.c
#clib/time.c

txn/txn.c
txn/txn_auto.c
txn/txn_autop.c
txn/txn_chkpt.c
txn/txn_failchk.c
txn/txn_method.c
txn/txn_rec.c
txn/txn_recover.c
txn/txn_region.c
txn/txn_stat.c
txn/txn_util.c

)

include_directories(.)

add_library(berkeleydb STATIC ${DB_PLATFORM_HEADERS} ${DB_PLATFORM_SRC} ${DB_SRC})

if(WIN32)

target_include_directories(berkeleydb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/build_windows )

endif()