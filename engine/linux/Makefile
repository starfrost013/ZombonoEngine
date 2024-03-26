#
# Zombono Makefile for Linux big.number
# 64-bit version
#
# Nov '97 by Zoid <zoid@idsoftware.com>
# Jan '19 by Krzysztof Kondrak <krzysztof.kondrak@gmail.com>
# Dec '23-Jan '24 by starfrost
#
# Mar 10, 2024: Remove all sorts of legacy nonsense

# ELF only
#

VERSION=0.05
VERSION_FN=$(VERSION)$(GLIBC)
RPM_RELEASE=6

ifneq (,$(findstring libc6,$(shell if [ -e /lib/libc.so.6* ];then echo libc6;fi)))
GLIBC=-glibc
else
GLIBC=
endif

KERNEL_ARCH=$(shell uname -m)

ifeq ($(KERNEL_ARCH),x86_64)
  ARCH=x64
  RPMARCH=i386
else ifeq ($(KERNEL_ARCH),amd64)
  ARCH=x64
  RPMARCH=i386
else ifeq ($(KERNEL_ARCH),armv7l)
  ARCH=armv7l
  RPMARCH=armv7l
else ifeq ($(KERNEL_ARCH),aarch64)
  ARCH=aarch64
  RPMARCH=aarch64
else
  $(error "Unsupported ARCH: ${KERNEL_ARCH}")
endif

NOARCH=noarch

MOUNT_DIR=..
VWEP_DIR=/grog/Projects/zombono/vwep
ZOMBONO_DIR=/grog/Projects/ZombonoMaster

BUILD_DEBUG_DIR=debug$(ARCH)$(GLIBC)
BUILD_RELEASE_DIR=release$(ARCH)$(GLIBC)
CLIENT_DIR=$(MOUNT_DIR)/client
SERVER_DIR=$(MOUNT_DIR)/server
REF_GL_DIR=$(MOUNT_DIR)/ref_gl
COMMON_DIR=$(MOUNT_DIR)/qcommon
LINUX_DIR=$(MOUNT_DIR)/linux
GAME_DIR=$(MOUNT_DIR)/game
NULL_DIR=$(MOUNT_DIR)/null
TOOLS_DIR=$(MOUNT_DIR)/../tools 

MESA_DIR=/usr/local/src/Mesa-2.6

CC=gcc
BASE_CFLAGS=-I/usr/local/include -I../client -I../game -I../qcommon -I../ref_gl -I../server -Dstricmp=strcasecmp -D_GNU_SOURCE -Wno-format-truncation -Wno-unused-result -Wno-format-overflow

RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG -O3 -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations

DEBUG_CFLAGS=$(BASE_CFLAGS) -g -D_DEBUG
LDFLAGS=-L/usr/local/lib -L$(MOUNT_DIR)/lib/linux -ldl -lm -lpthread -llibcurl
XCFLAGS=

GLLDFLAGS=-L/usr/X11R6/lib -L/usr/local/lib -L$(MESA_DIR)/lib -lX11 -lXext -lvga -lm
GLXLDFLAGS=-L/usr/X11R6/lib -L/usr/local/lib -L$(MESA_DIR)/lib -lX11 -lXext -lXxf86dga -lXxf86vm -lm

GLCFLAGS=-I$(MESA_DIR)/include -I/usr/include/glide
SHLIBEXT=so

SHLIBCFLAGS=-fPIC -std=c99
SHLIBCPPFLAGS=-fPIC -std=c++11
SHLIBLDFLAGS=-shared

DO_CC=$(CC) $(CFLAGS) -o $@ -c $<
DO_DED_CC=$(CC) $(CFLAGS) -DDEDICATED_ONLY -o $@ -c $<
DO_O_CC=$(CC) $(CFLAGS) -O -o $@ -c $<
DO_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_SHLIB_O_CC=$(CC) $(CFLAGS) -O $(SHLIBCFLAGS) -o $@ -c $<
DO_GL_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) $(GLCFLAGS) -o $@ -c $<
DO_GL_SHLIB_O_CC=$(CC) $(CFLAGS) -O $(SHLIBCFLAGS) $(GLCFLAGS) -o $@ -c $<
DO_AS=$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<
DO_SHLIB_AS=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<

#############################################################################
# SETUP AND BUILD
#############################################################################

TARGETS=$(BUILDDIR)/zombono \
	$(BUILDDIR)/ZombonoDedicatedServer \
	$(BUILDDIR)/zombonogame/game$(ARCH).$(SHLIBEXT) \
	$(BUILDDIR)/ref_glx.$(SHLIBEXT) \
	$(BUILDDIR)/ref_gl.$(SHLIBEXT)

build_debug:
	@-$(TOOLS_DIR)/autoincrement.sh

	@-mkdir -p $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)/zombonogame \
		$(BUILD_DEBUG_DIR)/client \
		$(BUILD_DEBUG_DIR)/ded \
		$(BUILD_DEBUG_DIR)/ref_gl \
		$(BUILD_DEBUG_DIR)/game \
		$(BUILD_DEBUG_DIR)/game/ammunition \
		$(BUILD_DEBUG_DIR)/game/entities \
		$(BUILD_DEBUG_DIR)/game/mobs \
		$(BUILD_DEBUG_DIR)/game/physics \
		$(BUILD_DEBUG_DIR)/game/weapons \
		$(BUILD_DEBUG_DIR)/qcommon \
		$(BUILD_DEBUG_DIR)/qcommon/netservices
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

build_release:
	@-$(TOOLS_DIR)/autoincrement.sh

	@-mkdir -p $(BUILD_RELEASE_DIR) \
		$(BUILD_RELEASE_DIR)/zombonogame \
		$(BUILD_RELEASE_DIR)/client \
		$(BUILD_RELEASE_DIR)/ded \
		$(BUILD_RELEASE_DIR)/ref_gl \
		$(BUILD_RELEASE_DIR)/game \
		$(BUILD_RELEASE_DIR)/game/ammunition \
		$(BUILD_RELEASE_DIR)/game/entities \
		$(BUILD_RELEASE_DIR)/game/mobs \
		$(BUILD_RELEASE_DIR)/game/physics \
		$(BUILD_RELEASE_DIR)/game/weapons \
		$(BUILD_DEBUG_DIR)/qcommon \
		$(BUILD_DEBUG_DIR)/qcommon/netservices
	$(MAKE) targets BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"

all: build_debug build_release
debug: build_debug
release: build_release

targets: $(TARGETS)

#############################################################################
# CLIENT/SERVER
#############################################################################

ZOMBONO_OBJS = \
	$(BUILDDIR)/qcommon/pdjson.o \
	$(BUILDDIR)/qcommon/common.o \
	$(BUILDDIR)/client/cl_ents.o \
	$(BUILDDIR)/client/cl_font.o \
	$(BUILDDIR)/client/cl_fx.o \
	$(BUILDDIR)/client/cl_input.o \
	$(BUILDDIR)/client/cl_inv.o \
	$(BUILDDIR)/client/cl_main.o \
	$(BUILDDIR)/client/cl_newfx.o \
	$(BUILDDIR)/client/cl_parse.o \
	$(BUILDDIR)/client/cl_pred.o \
	$(BUILDDIR)/client/cl_tent.o \
	$(BUILDDIR)/client/cl_text.o \
	$(BUILDDIR)/client/cl_scrn.o \
	$(BUILDDIR)/client/cl_ui.o \
	$(BUILDDIR)/client/cl_ui_eventhandling.o \
	$(BUILDDIR)/client/cl_ui_leaderboard.o \
	$(BUILDDIR)/client/cl_ui_scripts.o \
	$(BUILDDIR)/client/cl_view.o \
	$(BUILDDIR)/client/console.o \
	$(BUILDDIR)/client/keys.o \
	$(BUILDDIR)/client/menu.o \
	$(BUILDDIR)/client/snd_dma.o \
	$(BUILDDIR)/client/snd_mem.o \
	$(BUILDDIR)/client/snd_miniaudio.o \
	$(BUILDDIR)/client/snd_mix.o \
	$(BUILDDIR)/client/qmenu.o \
	$(BUILDDIR)/client/m_flash.o \
	\
	$(BUILDDIR)/qcommon/cmd.o \
	$(BUILDDIR)/qcommon/map_loader.o \
	\
	$(BUILDDIR)/qcommon/crc.o \
	$(BUILDDIR)/qcommon/cvar.o \
	$(BUILDDIR)/qcommon/files.o \
	$(BUILDDIR)/qcommon/md4.o \
	$(BUILDDIR)/qcommon/net_chan.o \
	$(BUILDDIR)/qcommon/netservices/netservices_base.o \
	$(BUILDDIR)/qcommon/netservices/netservices_update.o \
	\
	$(BUILDDIR)/client/sv_ccmds.o \
	$(BUILDDIR)/client/sv_ents.o \
	$(BUILDDIR)/client/sv_game.o \
	$(BUILDDIR)/client/sv_init.o \
	$(BUILDDIR)/client/sv_main.o \
	$(BUILDDIR)/client/sv_send.o \
	$(BUILDDIR)/client/sv_user.o \
	$(BUILDDIR)/client/sv_world.o \
	\
	$(BUILDDIR)/client/cd_linux.o \
	$(BUILDDIR)/client/q_shlinux.o \
	$(BUILDDIR)/client/vid_menu.o \
	$(BUILDDIR)/client/vid_so.o \
	$(BUILDDIR)/client/snd_linux.o \
	$(BUILDDIR)/client/sys_linux.o \
	$(BUILDDIR)/client/glob.o \
	$(BUILDDIR)/client/net_udp.o \
	\
	$(BUILDDIR)/client/q_shared.o \
	$(BUILDDIR)/client/pmove.o
	
$(BUILDDIR)/zombono : $(ZOMBONO_OBJS)
	$(CC) $(CFLAGS) -o $@ $(ZOMBONO_OBJS) $(LDFLAGS)

$(BUILDDIR)/qcommon/pdjson.o :    $(COMMON_DIR)/pdjson.c
	$(DO_CC)

$(BUILDDIR)/qcommon/common.o :     $(COMMON_DIR)/common.c
	$(DO_CC)
		
$(BUILDDIR)/qcommon/cmd.o :        $(COMMON_DIR)/cmd.c
	$(DO_CC)

$(BUILDDIR)/qcommon/map_loader.o :     $(COMMON_DIR)/map_loader.c
	$(DO_CC)

$(BUILDDIR)/qcommon/crc.o :        $(COMMON_DIR)/crc.c
	$(DO_CC)

$(BUILDDIR)/qcommon/cvar.o :       $(COMMON_DIR)/cvar.c
	$(DO_CC)

$(BUILDDIR)/qcommon/files.o :      $(COMMON_DIR)/files.c
	$(DO_CC)

$(BUILDDIR)/qcommon/md4.o :        $(COMMON_DIR)/md4.c
	$(DO_CC)

$(BUILDDIR)/qcommon/net_chan.o :   $(COMMON_DIR)/net_chan.c
	$(DO_CC)

$(BUILDDIR)/qcommon/netservices/netservices_base.o :   $(COMMON_DIR)/netservices/netservices_base.c
	$(DO_CC)
	
$(BUILDDIR)/qcommon/netservices/netservices_update.o :   $(COMMON_DIR)/netservices/netservices_update.c
	$(DO_CC)
	
$(BUILDDIR)/client/cl_ents.o :    $(CLIENT_DIR)/cl_ents.c
	$(DO_CC)

$(BUILDDIR)/client/cl_font.o :      $(CLIENT_DIR)/cl_font.c
	$(DO_CC)

$(BUILDDIR)/client/cl_fx.o :      $(CLIENT_DIR)/cl_fx.c
	$(DO_CC)

$(BUILDDIR)/client/cl_input.o :   $(CLIENT_DIR)/cl_input.c
	$(DO_CC)

$(BUILDDIR)/client/cl_inv.o :     $(CLIENT_DIR)/cl_inv.c
	$(DO_CC)

$(BUILDDIR)/client/cl_main.o :    $(CLIENT_DIR)/cl_main.c
	$(DO_CC)

$(BUILDDIR)/client/cl_newfx.o :    $(CLIENT_DIR)/cl_newfx.c
	$(DO_CC)

$(BUILDDIR)/client/cl_parse.o :   $(CLIENT_DIR)/cl_parse.c
	$(DO_CC)

$(BUILDDIR)/client/cl_pred.o :    $(CLIENT_DIR)/cl_pred.c
	$(DO_CC)

$(BUILDDIR)/client/cl_tent.o :    $(CLIENT_DIR)/cl_tent.c
	$(DO_CC)

$(BUILDDIR)/client/cl_text.o :    $(CLIENT_DIR)/cl_text.c
	$(DO_CC)
	
$(BUILDDIR)/client/cl_scrn.o :    $(CLIENT_DIR)/cl_scrn.c
	$(DO_CC)

$(BUILDDIR)/client/cl_ui.o :    $(CLIENT_DIR)/cl_ui.c
	$(DO_CC)
	
$(BUILDDIR)/client/cl_ui_eventhandling.o :    $(CLIENT_DIR)/cl_ui_eventhandling.c
	$(DO_CC)
	
$(BUILDDIR)/client/cl_ui_leaderboard.o :    $(CLIENT_DIR)/cl_ui_leaderboard.c
	$(DO_CC)
	
$(BUILDDIR)/client/cl_ui_scripts.o :    $(CLIENT_DIR)/cl_ui_scripts.c
	$(DO_CC)
	

$(BUILDDIR)/client/cl_view.o :    $(CLIENT_DIR)/cl_view.c
	$(DO_CC)

$(BUILDDIR)/client/console.o :    $(CLIENT_DIR)/console.c
	$(DO_CC)

$(BUILDDIR)/client/keys.o :       $(CLIENT_DIR)/keys.c
	$(DO_CC)

$(BUILDDIR)/client/menu.o :       $(CLIENT_DIR)/menu.c
	$(DO_CC)

$(BUILDDIR)/client/snd_dma.o :    $(CLIENT_DIR)/snd_dma.c
	$(DO_CC)

$(BUILDDIR)/client/snd_mem.o :    $(CLIENT_DIR)/snd_mem.c
	$(DO_CC)

$(BUILDDIR)/client/snd_miniaudio.o : $(CLIENT_DIR)/snd_miniaudio.c
	$(DO_CC)

$(BUILDDIR)/client/snd_mix.o :    $(CLIENT_DIR)/snd_mix.c
	$(DO_CC)

$(BUILDDIR)/client/qmenu.o :      $(CLIENT_DIR)/qmenu.c
	$(DO_CC)

$(BUILDDIR)/client/m_flash.o :    $(GAME_DIR)/m_flash.c
	$(DO_CC)



$(BUILDDIR)/client/q_shared.o :   $(GAME_DIR)/q_shared.c
	$(DO_CC)

$(BUILDDIR)/client/pmove.o :      $(COMMON_DIR)/pmove.c
	$(DO_CC)

$(BUILDDIR)/client/sv_ccmds.o :   $(SERVER_DIR)/sv_ccmds.c
	$(DO_CC)

$(BUILDDIR)/client/sv_ents.o :    $(SERVER_DIR)/sv_ents.c
	$(DO_CC)

$(BUILDDIR)/client/sv_game.o :    $(SERVER_DIR)/sv_game.c
	$(DO_CC)

$(BUILDDIR)/client/sv_init.o :    $(SERVER_DIR)/sv_init.c
	$(DO_CC)

$(BUILDDIR)/client/sv_main.o :    $(SERVER_DIR)/sv_main.c
	$(DO_CC)

$(BUILDDIR)/client/sv_send.o :    $(SERVER_DIR)/sv_send.c
	$(DO_CC)

$(BUILDDIR)/client/sv_user.o :    $(SERVER_DIR)/sv_user.c
	$(DO_CC)

$(BUILDDIR)/client/sv_world.o :   $(SERVER_DIR)/sv_world.c
	$(DO_CC)

$(BUILDDIR)/client/cd_linux.o :   $(LINUX_DIR)/cd_linux.c
	$(DO_CC)

$(BUILDDIR)/client/q_shlinux.o :  $(LINUX_DIR)/q_shlinux.c
	$(DO_O_CC)

$(BUILDDIR)/client/vid_menu.o :   $(LINUX_DIR)/vid_menu.c
	$(DO_CC)

$(BUILDDIR)/client/vid_so.o :     $(LINUX_DIR)/vid_so.c
	$(DO_CC)

$(BUILDDIR)/client/snd_linux.o :  $(LINUX_DIR)/snd_linux.c
	$(DO_CC)

$(BUILDDIR)/client/sys_linux.o :  $(LINUX_DIR)/sys_linux.c
	$(DO_CC)

$(BUILDDIR)/client/glob.o :       $(LINUX_DIR)/glob.c
	$(DO_CC)

$(BUILDDIR)/client/net_udp.o :    $(LINUX_DIR)/net_udp.c
	$(DO_CC)

#############################################################################
# DEDICATED SERVER
#############################################################################

ZombonoDedicatedServer_OBJS = \
	\
	$(BUILDDIR)/ded/cmd.o \
	$(BUILDDIR)/ded/map_loader.o \
	$(BUILDDIR)/ded/common.o \
	$(BUILDDIR)/ded/crc.o \
	$(BUILDDIR)/ded/cvar.o \
	$(BUILDDIR)/ded/files.o \
	$(BUILDDIR)/ded/md4.o \
	$(BUILDDIR)/ded/net_chan.o \
	\
	$(BUILDDIR)/ded/sv_ccmds.o \
	$(BUILDDIR)/ded/sv_ents.o \
	$(BUILDDIR)/ded/sv_game.o \
	$(BUILDDIR)/ded/sv_init.o \
	$(BUILDDIR)/ded/sv_main.o \
	$(BUILDDIR)/ded/sv_send.o \
	$(BUILDDIR)/ded/sv_user.o \
	$(BUILDDIR)/ded/sv_world.o \
	\
	$(BUILDDIR)/ded/q_shlinux.o \
	$(BUILDDIR)/ded/sys_linux.o \
	$(BUILDDIR)/ded/glob.o \
	$(BUILDDIR)/ded/net_udp.o \
	\
	$(BUILDDIR)/ded/q_shared.o \
	$(BUILDDIR)/ded/pmove.o \
	\
	$(BUILDDIR)/ded/cl_null.o \
	$(BUILDDIR)/ded/cd_null.o

$(BUILDDIR)/ZombonoDedicatedServer : $(ZombonoDedicatedServer_OBJS)
	$(CC) $(CFLAGS) -o $@ $(ZombonoDedicatedServer_OBJS) $(LDFLAGS)

$(BUILDDIR)/ded/cmd.o :        $(COMMON_DIR)/cmd.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/map_loader.o :     $(COMMON_DIR)/map_loader.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/common.o :     $(COMMON_DIR)/common.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/crc.o :        $(COMMON_DIR)/crc.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/cvar.o :       $(COMMON_DIR)/cvar.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/files.o :      $(COMMON_DIR)/files.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/md4.o :        $(COMMON_DIR)/md4.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/net_chan.o :   $(COMMON_DIR)/net_chan.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/q_shared.o :   $(GAME_DIR)/q_shared.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/pmove.o :      $(COMMON_DIR)/pmove.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_ccmds.o :   $(SERVER_DIR)/sv_ccmds.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_ents.o :    $(SERVER_DIR)/sv_ents.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_game.o :    $(SERVER_DIR)/sv_game.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_init.o :    $(SERVER_DIR)/sv_init.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_main.o :    $(SERVER_DIR)/sv_main.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_send.o :    $(SERVER_DIR)/sv_send.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_user.o :    $(SERVER_DIR)/sv_user.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sv_world.o :   $(SERVER_DIR)/sv_world.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/q_shlinux.o :  $(LINUX_DIR)/q_shlinux.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/sys_linux.o :  $(LINUX_DIR)/sys_linux.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/glob.o :       $(LINUX_DIR)/glob.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/net_udp.o :    $(LINUX_DIR)/net_udp.c
	$(DO_DED_CC)

$(BUILDDIR)/ded/cd_null.o     : $(NULL_DIR)/cd_null.c    
	$(DO_DED_CC)

$(BUILDDIR)/ded/cl_null.o     : $(NULL_DIR)/cl_null.c    
	$(DO_DED_CC)

#############################################################################
# GAME
#############################################################################

GAME_OBJS = \
	$(BUILDDIR)/game/q_shared.o \
	$(BUILDDIR)/game/g_ai.o \
	$(BUILDDIR)/game/p_client.o \
	$(BUILDDIR)/game/g_cmds.o \
	$(BUILDDIR)/game/g_svcmds.o \
	$(BUILDDIR)/game/g_chase.o \
	$(BUILDDIR)/game/g_combat.o \

	$(BUILDDIR)/game/g_main.o \

	$(BUILDDIR)/game/g_monster.o \
	$(BUILDDIR)/game/g_phys.o \
	$(BUILDDIR)/game/g_save.o \
	$(BUILDDIR)/game/g_spawn.o \
	$(BUILDDIR)/game/g_utils.o \
	$(BUILDDIR)/game/ammunition/ammo_bamfuslicator.o \
	$(BUILDDIR)/game/ammunition/ammo_bfg.o \
	$(BUILDDIR)/game/ammunition/ammo_blaster.o \
	$(BUILDDIR)/game/ammunition/ammo_bullet.o \
	$(BUILDDIR)/game/ammunition/ammo_grenade.o \
	$(BUILDDIR)/game/ammunition/ammo_melee.o \
	$(BUILDDIR)/game/ammunition/ammo_rail.o \
	$(BUILDDIR)/game/ammunition/ammo_rocket.o \
	$(BUILDDIR)/game/entities/entity_func.o \
	$(BUILDDIR)/game/entities/entity_items.o \
	$(BUILDDIR)/game/entities/entity_misc.o \
	$(BUILDDIR)/game/entities/entity_target.o \
	$(BUILDDIR)/game/entities/entity_trigger.o \
	$(BUILDDIR)/game/mobs/mob_ogre.o \
	$(BUILDDIR)/game/mobs/mob_soldier.o \
	$(BUILDDIR)/game/mobs/mob_zombie.o \
	$(BUILDDIR)/game/mobs/mob_zombie_fast.o \
	$(BUILDDIR)/game/physics/physics_base.o \
	$(BUILDDIR)/game/physics/physics_movement.o \
	$(BUILDDIR)/game/weapons/weapon_bamfuslicator.o \
	$(BUILDDIR)/game/weapons/weapon_base.o \
	$(BUILDDIR)/game/weapons/weapon_bfg.o \
	$(BUILDDIR)/game/weapons/weapon_blaster.o \
	$(BUILDDIR)/game/weapons/weapon_blaster_hyper.o \
	$(BUILDDIR)/game/weapons/weapon_chaingun.o \
	$(BUILDDIR)/game/weapons/weapon_grenade_hand.o \
	$(BUILDDIR)/game/weapons/weapon_grenade_launcher.o \
	$(BUILDDIR)/game/weapons/weapon_machinegun.o \
	$(BUILDDIR)/game/weapons/weapon_railgun.o \
	$(BUILDDIR)/game/weapons/weapon_rocket_launcher.o \
	$(BUILDDIR)/game/weapons/weapon_shotgun.o \
	$(BUILDDIR)/game/weapons/weapon_shotgun_super.o \
	$(BUILDDIR)/game/p_hud.o \
	$(BUILDDIR)/game/p_trail.o \
	$(BUILDDIR)/game/p_view.o \
	$(BUILDDIR)/game/m_flash.o

$(BUILDDIR)/zombonogame/game$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS)

$(BUILDDIR)/game/g_ai.o :        $(GAME_DIR)/g_ai.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/p_client.o :    $(GAME_DIR)/p_client.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_cmds.o :      $(GAME_DIR)/g_cmds.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_svcmds.o :    $(GAME_DIR)/g_svcmds.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_chase.o :    $(GAME_DIR)/g_chase.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_combat.o :    $(GAME_DIR)/g_combat.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_main.o :      $(GAME_DIR)/g_main.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_monster.o :   $(GAME_DIR)/g_monster.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_phys.o :      $(GAME_DIR)/g_phys.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_save.o :      $(GAME_DIR)/g_save.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_spawn.o :     $(GAME_DIR)/g_spawn.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/g_utils.o :     $(GAME_DIR)/g_utils.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/ammunition/ammo_bamfuslicator.o :    $(GAME_DIR)/ammunition/ammo_bamfuslicator.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/ammunition/ammo_bfg.o :    $(GAME_DIR)/ammunition/ammo_bfg.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/ammunition/ammo_blaster.o :    $(GAME_DIR)/ammunition/ammo_blaster.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/ammunition/ammo_bullet.o :    $(GAME_DIR)/ammunition/ammo_bullet.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/ammunition/ammo_grenade.o :    $(GAME_DIR)/ammunition/ammo_grenade.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/ammunition/ammo_melee.o :    $(GAME_DIR)/ammunition/ammo_melee.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/ammunition/ammo_rail.o :    $(GAME_DIR)/ammunition/ammo_rail.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/ammunition/ammo_rocket.o :    $(GAME_DIR)/ammunition/ammo_rocket.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/entities/entity_func.o :      $(GAME_DIR)/entities/entity_func.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/entities/entity_items.o :     $(GAME_DIR)/entities/entity_items.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/entities/entity_misc.o :      $(GAME_DIR)/entities/entity_misc.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/entities/entity_target.o :    $(GAME_DIR)/entities/entityg_target.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/entities/entity_trigger.o :   $(GAME_DIR)/entities/entity_trigger.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/mobs/mob_ogre.o :   	$(GAME_DIR)/mobs/mob_ogre.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/mobs/mob_soldier.o :   $(GAME_DIR)/mobs/mob_soldier.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/mobs/mob_zombie.o :   $(GAME_DIR)/mobs/mob_zombie.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/mobs/mob_zombie_fast.o :   $(GAME_DIR)/mobs/mob_zombie_fast.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/physics/physics_base.o :   $(GAME_DIR)/physics/physics_base.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/physics/physics_movement.o :   $(GAME_DIR)/physics/physics_movement.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/weapons/weapon_bamfuslicator.o :    $(GAME_DIR)/weapons/weapon_bamfuslicator.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/weapons/weapon_base.o :    $(GAME_DIR)/weapons/weapon_base.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/weapons/weapon_bfg.o :    $(GAME_DIR)/weapons/weapon_bfg.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/weapons/weapon_blaster.o :    $(GAME_DIR)/weapons/weapon_blaster.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/weapons/weapon_blaster_hyper.o :    $(GAME_DIR)/weapons/weapon_blaster_hyper.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/weapons/weapon_chaingun.o :    $(GAME_DIR)/weapons/weapon_chaingun.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/weapons/weapon_grenade_hand.o :    $(GAME_DIR)/weapons/weapon_grenade_hand.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/weapons/weapon_grenade_launcher.o :    $(GAME_DIR)/weapons/weapon_grenade_launcher.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/weapons/weapon_machinegun.o :    $(GAME_DIR)/weapons/weapon_machinegun.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/weapons/weapon_railgun.o :    $(GAME_DIR)/weapons/weapon_railgun.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/weapons/weapon_rocket_launcher.o :    $(GAME_DIR)/weapons/weapon_rocket_launcher.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/weapons/weapon_shotgun.o :    $(GAME_DIR)/weapons/weapon_shotgun.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/weapons/weapon_shotgun_super.o :    $(GAME_DIR)/weapons/weapon_shotgun_super.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/p_hud.o :       $(GAME_DIR)/p_hud.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/p_trail.o :     $(GAME_DIR)/p_trail.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/p_view.o :      $(GAME_DIR)/p_view.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/q_shared.o :    $(GAME_DIR)/q_shared.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/game/m_flash.o :     $(GAME_DIR)/m_flash.c
	$(DO_SHLIB_CC)

#############################################################################
# REF_GL
#############################################################################

REF_GL_OBJS = \
	$(BUILDDIR)/ref_gl/gl_draw.o \
	$(BUILDDIR)/ref_gl/gl_image.o \
	$(BUILDDIR)/ref_gl/gl_light.o \
	$(BUILDDIR)/ref_gl/gl_mesh.o \
	$(BUILDDIR)/ref_gl/gl_model.o \
	$(BUILDDIR)/ref_gl/gl_rmain.o \
	$(BUILDDIR)/ref_gl/gl_rmisc.o \
	$(BUILDDIR)/ref_gl/gl_rsurf.o \
	$(BUILDDIR)/ref_gl/gl_sprite.o \
	$(BUILDDIR)/ref_gl/gl_warp.o \
	\
	$(BUILDDIR)/ref_gl/qgl_linux.o \
	$(BUILDDIR)/ref_gl/q_shared.o \
	$(BUILDDIR)/ref_gl/q_shlinux.o \
	$(BUILDDIR)/ref_gl/glob.o

REF_GL_FXMESA_OBJS = \
	$(BUILDDIR)/ref_gl/gl_fxmesa.o \

REF_GL_GLX_OBJS = \
	$(BUILDDIR)/ref_gl/gl_glx.o

$(BUILDDIR)/ref_gl.$(SHLIBEXT) : $(REF_GL_OBJS) $(REF_GL_FXMESA_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(REF_GL_OBJS) $(REF_GL_FXMESA_OBJS) $(GLLDFLAGS)

$(BUILDDIR)/ref_glx.$(SHLIBEXT) : $(REF_GL_OBJS) $(REF_GL_GLX_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(REF_GL_OBJS) $(REF_GL_GLX_OBJS) $(GLXLDFLAGS)

$(BUILDDIR)/ref_gl/gl_draw.o :        $(REF_GL_DIR)/gl_draw.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/gl_image.o :       $(REF_GL_DIR)/gl_image.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/gl_light.o :       $(REF_GL_DIR)/gl_light.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/gl_mesh.o :        $(REF_GL_DIR)/gl_mesh.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/gl_model.o :       $(REF_GL_DIR)/gl_model.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/gl_rmain.o :       $(REF_GL_DIR)/gl_rmain.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/gl_rmisc.o :       $(REF_GL_DIR)/gl_rmisc.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/gl_rsurf.o :       $(REF_GL_DIR)/gl_rsurf.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/gl_sprite.o :        $(REF_GL_DIR)/gl_sprite.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/gl_warp.o :        $(REF_GL_DIR)/gl_warp.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/qgl_linux.o :      $(LINUX_DIR)/qgl_linux.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/gl_fxmesa.o :      $(LINUX_DIR)/gl_fxmesa.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/gl_glx.o :      $(LINUX_DIR)/gl_glx.c
	$(DO_GL_SHLIB_CC)

$(BUILDDIR)/ref_gl/q_shared.o :       $(GAME_DIR)/q_shared.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/ref_gl/q_shlinux.o :      $(LINUX_DIR)/q_shlinux.c
	$(DO_GL_SHLIB_O_CC)

$(BUILDDIR)/ref_gl/glob.o :           $(LINUX_DIR)/glob.c
	$(DO_GL_SHLIB_CC)

#############################################################################
# RPM
#############################################################################

# Make RPMs.  You need to be root to make this work
RPMROOT=/usr/src/redhat
RPM = rpm
RPMFLAGS = -bb
INSTALLDIR = /usr/local/games/zombono
TDFXGL_DIR = /home/zoid/3dfxgl
TMPDIR = /var/tmp

ifeq ($(ARCH),axp)

RPMDIR = $(TMPDIR)/ZombonoDedicatedServer-$(VERSION_FN)
DESTDIR= $(RPMDIR)/$(INSTALLDIR)

rpm: zombono.axp.spec
	touch $(RPMROOT)/SOURCES/ZombonoDedicatedServer-$(VERSION_FN).tar.gz
	if [ ! -d archives ];then mkdir archives;fi
	$(MAKE) copyfiles COPYDIR=$(DESTDIR)
	cp $(MOUNT_DIR)/linux/zombono.gif $(RPMROOT)/SOURCES/.
	cp zombono.axp.spec $(RPMROOT)/SPECS/.
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) zombono.axp.spec
	rm -rf $(RPMDIR)
	mv $(RPMROOT)/RPMS/$(RPMARCH)/ZombonoDedicatedServer-$(VERSION_FN)-$(RPM_RELEASE).$(RPMARCH).rpm archives/.

tar:
	if [ ! -d archives ];then mkdir archives;fi
	$(MAKE) copyfiles COPYDIR=$(RPMDIR)
	cd $(RPMDIR); tar czvf ZombonoDedicatedServer-$(VERSION_FN)-$(RPMARCH)-unknown-linux2.0.tar.gz *
	mv $(RPMDIR)/*.tar.gz archives/.
	rm -rf $(RPMDIR)

copyfiles:
	-mkdirhier $(COPYDIR)
	cp $(BUILD_RELEASE_DIR)/ZombonoDedicatedServer $(COPYDIR)
	strip $(COPYDIR)/ZombonoDedicatedServer
	chmod 755 $(COPYDIR)/ZombonoDedicatedServer
	-mkdir $(COPYDIR)/zombonogame
	chmod 755 $(COPYDIR)/zombonogame
	cp $(BUILD_RELEASE_DIR)/game$(ARCH).$(SHLIBEXT) $(COPYDIR)/zombonogame
	chmod 755 $(COPYDIR)/zombonogame/game$(ARCH).$(SHLIBEXT)
	cp $(MOUNT_DIR)/readme.txt $(COPYDIR)
	cp $(MOUNT_DIR)/$(VERSION)_Changes.txt $(COPYDIR)

else

# rpm-zombono-patch
rpm:   rpm-zombono rpm-zombono-data rpm-zombono-video \
		rpm-zombono-ctf

PATCHDIR = $(TMPDIR)/zombono-patch-$(VERSION_FN)
rpm-zombono-patch: zombono-patch.spec
	touch $(RPMROOT)/SOURCES/zombono-patch-$(VERSION_FN).tar.gz
	if [ ! -d archives ];then mkdir archives;fi
	$(MAKE) copyfiles COPYDIR=$(PATCHDIR)/$(INSTALLDIR)
	-mkdir $(RPMDIR)/etc
	cp $(MOUNT_DIR)/linux/zombono.conf $(PATCHDIR)/etc/zombono.conf
	cp $(MOUNT_DIR)/linux/zombono.gif $(RPMROOT)/SOURCES/.
	cp $< $(RPMROOT)/SPECS/.
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) $<
	rm -rf $(PATCHDIR)
	mv $(RPMROOT)/RPMS/$(ARCH)/zombono-$(VERSION_FN)-$(RPM_RELEASE).$(ARCH).rpm archives/.

ZOMBONO_DIR = $(TMPDIR)/zombono-$(VERSION_FN)
ZOMBONO_INSTALL_DIR = $(TMPDIR)/zombono-$(VERSION_FN)/$(INSTALLDIR)
rpm-zombono: zombono.spec
	touch $(RPMROOT)/SOURCES/zombono-$(VERSION_FN).tar.gz
	if [ ! -d archives ];then mkdir archives;fi
	-mkdirhier $(ZOMBONO_INSTALL_DIR)
	cp $(BUILD_RELEASE_DIR)/zombono $(ZOMBONO_INSTALL_DIR)/zombono
	strip $(ZOMBONO_INSTALL_DIR)/zombono
	-mkdir $(ZOMBONO_INSTALL_DIR)/zombonogame
	chmod 1777 $(ZOMBONO_INSTALL_DIR)/zombonogame
	cp $(BUILD_RELEASE_DIR)/game$(ARCH).$(SHLIBEXT) $(ZOMBONO_INSTALL_DIR)/zombonogame
	cp $(BUILD_RELEASE_DIR)/ref_gl.$(SHLIBEXT) $(ZOMBONO_INSTALL_DIR)
	cp $(BUILD_RELEASE_DIR)/ref_glx.$(SHLIBEXT) $(ZOMBONO_INSTALL_DIR)
	-mkdirhier $(ZOMBONO_DIR)/usr/lib
	cp $(TDFXGL_DIR)/release$(ARCH)$(GLIBC)/lib3dfxgl.so $(ZOMBONO_DIR)/usr/lib/lib3dfxgl.so
	cp $(MESA_DIR)/lib/libMesaGL.so.2.6 $(ZOMBONO_DIR)/usr/lib/libMesaGL.so.2.6
	cp $(MOUNT_DIR)/linux/README $(ZOMBONO_INSTALL_DIR)
	cp $(MOUNT_DIR)/readme.txt $(ZOMBONO_INSTALL_DIR)
	-mkdir $(ZOMBONO_DIR)/etc
	cp $(MOUNT_DIR)/linux/zombono.conf $(ZOMBONO_DIR)/etc/zombono.conf
	cp $(MOUNT_DIR)/linux/zombono.gif $(RPMROOT)/SOURCES/.
	cp $< $(RPMROOT)/SPECS/.
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) $<
	rm -rf $(ZOMBONO_DIR)
	mv $(RPMROOT)/RPMS/$(ARCH)/zombono-$(VERSION_FN)-$(RPM_RELEASE).$(ARCH).rpm archives/.

# rpm-zombono-patch: zombono-patch.spec

zombonoDATADIR = $(TMPDIR)/zombono-data-$(VERSION)/$(INSTALLDIR)
rpm-zombono-data: zombono-data.spec
	touch $(RPMROOT)/SOURCES/zombono-data-$(VERSION).tar.gz
	if [ ! -d archives ];then mkdir archives;fi
	-mkdirhier $(zombonoDATADIR)/zombonogame
	-mkdirhier $(zombonoDATADIR)/zombonogame/players
	-mkdirhier $(zombonoDATADIR)/zombonogame/players/cyborg
	-mkdirhier $(zombonoDATADIR)/zombonogame/players/female
	-mkdirhier $(zombonoDATADIR)/zombonogame/players/male
	cp $(ZOMBONO_DIR)/zombonogame/pak0.pak							 	 $(zombonoDATADIR)/zombonogame/pak0.pak                         
	cp $(ZOMBONO_DIR)/zombonogame/pak1.pak                                 $(zombonoDATADIR)/zombonogame/pak1.pak                         
	cp $(ZOMBONO_DIR)/zombonogame/pak2.pak                                 $(zombonoDATADIR)/zombonogame/pak2.pak                         
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/a_grenades.md2            $(zombonoDATADIR)/zombonogame/players/cyborg/a_grenades.md2    
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_bfg.md2                 $(zombonoDATADIR)/zombonogame/players/cyborg/w_bfg.md2         
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_blaster.md2             $(zombonoDATADIR)/zombonogame/players/cyborg/w_blaster.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_chainfist.md2           $(zombonoDATADIR)/zombonogame/players/cyborg/w_chainfist.md2   
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_chaingun.md2            $(zombonoDATADIR)/zombonogame/players/cyborg/w_chaingun.md2    
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_disrupt.md2             $(zombonoDATADIR)/zombonogame/players/cyborg/w_disrupt.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_etfrifle.md2            $(zombonoDATADIR)/zombonogame/players/cyborg/w_etfrifle.md2    
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_glauncher.md2           $(zombonoDATADIR)/zombonogame/players/cyborg/w_glauncher.md2   
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_grapple.md2             $(zombonoDATADIR)/zombonogame/players/cyborg/w_grapple.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_hyperblaster.md2        $(zombonoDATADIR)/zombonogame/players/cyborg/w_hyperblaster.md2
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_machinegun.md2          $(zombonoDATADIR)/zombonogame/players/cyborg/w_machinegun.md2  
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_phalanx.md2             $(zombonoDATADIR)/zombonogame/players/cyborg/w_phalanx.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_plasma.md2              $(zombonoDATADIR)/zombonogame/players/cyborg/w_plasma.md2      
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_plauncher.md2           $(zombonoDATADIR)/zombonogame/players/cyborg/w_plauncher.md2   
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_railgun.md2             $(zombonoDATADIR)/zombonogame/players/cyborg/w_railgun.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_ripper.md2              $(zombonoDATADIR)/zombonogame/players/cyborg/w_ripper.md2      
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_rlauncher.md2           $(zombonoDATADIR)/zombonogame/players/cyborg/w_rlauncher.md2   
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_shotgun.md2             $(zombonoDATADIR)/zombonogame/players/cyborg/w_shotgun.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/w_sshotgun.md2            $(zombonoDATADIR)/zombonogame/players/cyborg/w_sshotgun.md2    
	cp $(ZOMBONO_DIR)/zombonogame/players/female/a_grenades.md2            $(zombonoDATADIR)/zombonogame/players/female/a_grenades.md2    
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_bfg.md2                 $(zombonoDATADIR)/zombonogame/players/female/w_bfg.md2         
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_blaster.md2             $(zombonoDATADIR)/zombonogame/players/female/w_blaster.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_chainfist.md2           $(zombonoDATADIR)/zombonogame/players/female/w_chainfist.md2   
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_chaingun.md2            $(zombonoDATADIR)/zombonogame/players/female/w_chaingun.md2    
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_disrupt.md2             $(zombonoDATADIR)/zombonogame/players/female/w_disrupt.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_etfrifle.md2            $(zombonoDATADIR)/zombonogame/players/female/w_etfrifle.md2    
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_glauncher.md2           $(zombonoDATADIR)/zombonogame/players/female/w_glauncher.md2   
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_grapple.md2             $(zombonoDATADIR)/zombonogame/players/female/w_grapple.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_hyperblaster.md2        $(zombonoDATADIR)/zombonogame/players/female/w_hyperblaster.md2
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_machinegun.md2          $(zombonoDATADIR)/zombonogame/players/female/w_machinegun.md2  
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_phalanx.md2             $(zombonoDATADIR)/zombonogame/players/female/w_phalanx.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_plasma.md2              $(zombonoDATADIR)/zombonogame/players/female/w_plasma.md2      
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_plauncher.md2           $(zombonoDATADIR)/zombonogame/players/female/w_plauncher.md2   
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_railgun.md2             $(zombonoDATADIR)/zombonogame/players/female/w_railgun.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_ripper.md2              $(zombonoDATADIR)/zombonogame/players/female/w_ripper.md2      
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_rlauncher.md2           $(zombonoDATADIR)/zombonogame/players/female/w_rlauncher.md2   
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_shotgun.md2             $(zombonoDATADIR)/zombonogame/players/female/w_shotgun.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/female/w_sshotgun.md2            $(zombonoDATADIR)/zombonogame/players/female/w_sshotgun.md2    
	cp $(ZOMBONO_DIR)/zombonogame/players/male/a_grenades.md2              $(zombonoDATADIR)/zombonogame/players/male/a_grenades.md2      
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_bfg.md2                   $(zombonoDATADIR)/zombonogame/players/male/w_bfg.md2           
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_blaster.md2               $(zombonoDATADIR)/zombonogame/players/male/w_blaster.md2       
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_chainfist.md2             $(zombonoDATADIR)/zombonogame/players/male/w_chainfist.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_chaingun.md2              $(zombonoDATADIR)/zombonogame/players/male/w_chaingun.md2      
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_disrupt.md2               $(zombonoDATADIR)/zombonogame/players/male/w_disrupt.md2       
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_etfrifle.md2              $(zombonoDATADIR)/zombonogame/players/male/w_etfrifle.md2      
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_glauncher.md2             $(zombonoDATADIR)/zombonogame/players/male/w_glauncher.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_grapple.md2               $(zombonoDATADIR)/zombonogame/players/male/w_grapple.md2       
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_hyperblaster.md2          $(zombonoDATADIR)/zombonogame/players/male/w_hyperblaster.md2  
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_machinegun.md2            $(zombonoDATADIR)/zombonogame/players/male/w_machinegun.md2    
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_phalanx.md2               $(zombonoDATADIR)/zombonogame/players/male/w_phalanx.md2       
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_plasma.md2                $(zombonoDATADIR)/zombonogame/players/male/w_plasma.md2        
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_plauncher.md2             $(zombonoDATADIR)/zombonogame/players/male/w_plauncher.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_railgun.md2               $(zombonoDATADIR)/zombonogame/players/male/w_railgun.md2       
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_ripper.md2                $(zombonoDATADIR)/zombonogame/players/male/w_ripper.md2        
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_rlauncher.md2             $(zombonoDATADIR)/zombonogame/players/male/w_rlauncher.md2     
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_shotgun.md2               $(zombonoDATADIR)/zombonogame/players/male/w_shotgun.md2       
	cp $(ZOMBONO_DIR)/zombonogame/players/male/w_sshotgun.md2              $(zombonoDATADIR)/zombonogame/players/male/w_sshotgun.md2      
	cp $(ZOMBONO_DIR)/zombonogame/players/female/athena.tga                $(zombonoDATADIR)/zombonogame/players/female/athena.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/female/athena_i.tga              $(zombonoDATADIR)/zombonogame/players/female/athena_i.tga      
	cp $(ZOMBONO_DIR)/zombonogame/players/female/brianna.tga               $(zombonoDATADIR)/zombonogame/players/female/brianna.tga       
	cp $(ZOMBONO_DIR)/zombonogame/players/female/brianna_i.tga             $(zombonoDATADIR)/zombonogame/players/female/brianna_i.tga     
	cp $(ZOMBONO_DIR)/zombonogame/players/female/cobalt.tga                $(zombonoDATADIR)/zombonogame/players/female/cobalt.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/female/cobalt_i.tga              $(zombonoDATADIR)/zombonogame/players/female/cobalt_i.tga      
	cp $(ZOMBONO_DIR)/zombonogame/players/female/ensign.tga                $(zombonoDATADIR)/zombonogame/players/female/ensign.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/female/ensign_i.tga              $(zombonoDATADIR)/zombonogame/players/female/ensign_i.tga      
	cp $(ZOMBONO_DIR)/zombonogame/players/female/jezebel.tga               $(zombonoDATADIR)/zombonogame/players/female/jezebel.tga       
	cp $(ZOMBONO_DIR)/zombonogame/players/female/jezebel_i.tga             $(zombonoDATADIR)/zombonogame/players/female/jezebel_i.tga     
	cp $(ZOMBONO_DIR)/zombonogame/players/female/jungle.tga                $(zombonoDATADIR)/zombonogame/players/female/jungle.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/female/jungle_i.tga              $(zombonoDATADIR)/zombonogame/players/female/jungle_i.tga      
	cp $(ZOMBONO_DIR)/zombonogame/players/female/lotus.tga                 $(zombonoDATADIR)/zombonogame/players/female/lotus.tga         
	cp $(ZOMBONO_DIR)/zombonogame/players/female/lotus_i.tga               $(zombonoDATADIR)/zombonogame/players/female/lotus_i.tga       
	cp $(ZOMBONO_DIR)/zombonogame/players/female/stiletto.tga              $(zombonoDATADIR)/zombonogame/players/female/stiletto.tga      
	cp $(ZOMBONO_DIR)/zombonogame/players/female/stiletto_i.tga            $(zombonoDATADIR)/zombonogame/players/female/stiletto_i.tga    
	cp $(ZOMBONO_DIR)/zombonogame/players/female/tris.md2                  $(zombonoDATADIR)/zombonogame/players/female/tris.md2          
	cp $(ZOMBONO_DIR)/zombonogame/players/female/venus.tga                 $(zombonoDATADIR)/zombonogame/players/female/venus.tga         
	cp $(ZOMBONO_DIR)/zombonogame/players/female/venus_i.tga               $(zombonoDATADIR)/zombonogame/players/female/venus_i.tga       
	cp $(ZOMBONO_DIR)/zombonogame/players/female/voodoo.tga                $(zombonoDATADIR)/zombonogame/players/female/voodoo.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/female/voodoo_i.tga              $(zombonoDATADIR)/zombonogame/players/female/voodoo_i.tga      
	cp $(ZOMBONO_DIR)/zombonogame/players/female/weapon.md2                $(zombonoDATADIR)/zombonogame/players/female/weapon.md2        
	cp $(ZOMBONO_DIR)/zombonogame/players/female/weapon.tga                $(zombonoDATADIR)/zombonogame/players/female/weapon.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/male/cipher.tga                  $(zombonoDATADIR)/zombonogame/players/male/cipher.tga          
	cp $(ZOMBONO_DIR)/zombonogame/players/male/cipher_i.tga                $(zombonoDATADIR)/zombonogame/players/male/cipher_i.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/male/claymore.tga                $(zombonoDATADIR)/zombonogame/players/male/claymore.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/male/claymore_i.tga              $(zombonoDATADIR)/zombonogame/players/male/claymore_i.tga      
	cp $(ZOMBONO_DIR)/zombonogame/players/male/flak.tga                    $(zombonoDATADIR)/zombonogame/players/male/flak.tga            
	cp $(ZOMBONO_DIR)/zombonogame/players/male/flak_i.tga                  $(zombonoDATADIR)/zombonogame/players/male/flak_i.tga          
	cp $(ZOMBONO_DIR)/zombonogame/players/male/grunt.tga                   $(zombonoDATADIR)/zombonogame/players/male/grunt.tga           
	cp $(ZOMBONO_DIR)/zombonogame/players/male/grunt_i.tga                 $(zombonoDATADIR)/zombonogame/players/male/grunt_i.tga         
	cp $(ZOMBONO_DIR)/zombonogame/players/male/howitzer.tga                $(zombonoDATADIR)/zombonogame/players/male/howitzer.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/male/howitzer_i.tga              $(zombonoDATADIR)/zombonogame/players/male/howitzer_i.tga      
	cp $(ZOMBONO_DIR)/zombonogame/players/male/major.tga                   $(zombonoDATADIR)/zombonogame/players/male/major.tga           
	cp $(ZOMBONO_DIR)/zombonogame/players/male/major_i.tga                 $(zombonoDATADIR)/zombonogame/players/male/major_i.tga         
	cp $(ZOMBONO_DIR)/zombonogame/players/male/nightops.tga                $(zombonoDATADIR)/zombonogame/players/male/nightops.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/male/nightops_i.tga              $(zombonoDATADIR)/zombonogame/players/male/nightops_i.tga      
	cp $(ZOMBONO_DIR)/zombonogame/players/male/pointman.tga                $(zombonoDATADIR)/zombonogame/players/male/pointman.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/male/pointman_i.tga              $(zombonoDATADIR)/zombonogame/players/male/pointman_i.tga      
	cp $(ZOMBONO_DIR)/zombonogame/players/male/psycho.tga                  $(zombonoDATADIR)/zombonogame/players/male/psycho.tga          
	cp $(ZOMBONO_DIR)/zombonogame/players/male/psycho_i.tga                $(zombonoDATADIR)/zombonogame/players/male/psycho_i.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/male/rampage.tga                 $(zombonoDATADIR)/zombonogame/players/male/rampage.tga         
	cp $(ZOMBONO_DIR)/zombonogame/players/male/rampage_i.tga               $(zombonoDATADIR)/zombonogame/players/male/rampage_i.tga       
	cp $(ZOMBONO_DIR)/zombonogame/players/male/razor.tga                   $(zombonoDATADIR)/zombonogame/players/male/razor.tga           
	cp $(ZOMBONO_DIR)/zombonogame/players/male/razor_i.tga                 $(zombonoDATADIR)/zombonogame/players/male/razor_i.tga         
	cp $(ZOMBONO_DIR)/zombonogame/players/male/recon.tga                   $(zombonoDATADIR)/zombonogame/players/male/recon.tga           
	cp $(ZOMBONO_DIR)/zombonogame/players/male/recon_i.tga                 $(zombonoDATADIR)/zombonogame/players/male/recon_i.tga         
	cp $(ZOMBONO_DIR)/zombonogame/players/male/scout.tga                   $(zombonoDATADIR)/zombonogame/players/male/scout.tga           
	cp $(ZOMBONO_DIR)/zombonogame/players/male/scout_i.tga                 $(zombonoDATADIR)/zombonogame/players/male/scout_i.tga         
	cp $(ZOMBONO_DIR)/zombonogame/players/male/sniper.tga                  $(zombonoDATADIR)/zombonogame/players/male/sniper.tga          
	cp $(ZOMBONO_DIR)/zombonogame/players/male/sniper_i.tga                $(zombonoDATADIR)/zombonogame/players/male/sniper_i.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/male/tris.md2                    $(zombonoDATADIR)/zombonogame/players/male/tris.md2            
	cp $(ZOMBONO_DIR)/zombonogame/players/male/viper.tga                   $(zombonoDATADIR)/zombonogame/players/male/viper.tga           
	cp $(ZOMBONO_DIR)/zombonogame/players/male/viper_i.tga                 $(zombonoDATADIR)/zombonogame/players/male/viper_i.tga         
	cp $(ZOMBONO_DIR)/zombonogame/players/male/weapon.md2                  $(zombonoDATADIR)/zombonogame/players/male/weapon.md2          
	cp $(ZOMBONO_DIR)/zombonogame/players/male/weapon.tga                  $(zombonoDATADIR)/zombonogame/players/male/weapon.tga          
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/bump1.wav                 $(zombonoDATADIR)/zombonogame/players/cyborg/bump1.wav         
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/death1.wav                $(zombonoDATADIR)/zombonogame/players/cyborg/death1.wav        
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/death2.wav                $(zombonoDATADIR)/zombonogame/players/cyborg/death2.wav        
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/death3.wav                $(zombonoDATADIR)/zombonogame/players/cyborg/death3.wav        
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/death4.wav                $(zombonoDATADIR)/zombonogame/players/cyborg/death4.wav        
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/drown1.wav                $(zombonoDATADIR)/zombonogame/players/cyborg/drown1.wav        
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/fall1.wav                 $(zombonoDATADIR)/zombonogame/players/cyborg/fall1.wav         
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/fall2.wav                 $(zombonoDATADIR)/zombonogame/players/cyborg/fall2.wav         
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/gurp1.wav                 $(zombonoDATADIR)/zombonogame/players/cyborg/gurp1.wav         
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/gurp2.wav                 $(zombonoDATADIR)/zombonogame/players/cyborg/gurp2.wav         
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/jump1.wav                 $(zombonoDATADIR)/zombonogame/players/cyborg/jump1.wav         
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/oni911.tga                $(zombonoDATADIR)/zombonogame/players/cyborg/oni911.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/oni911_i.tga              $(zombonoDATADIR)/zombonogame/players/cyborg/oni911_i.tga      
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/pain100_1.wav             $(zombonoDATADIR)/zombonogame/players/cyborg/pain100_1.wav     
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/pain100_2.wav             $(zombonoDATADIR)/zombonogame/players/cyborg/pain100_2.wav     
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/pain25_1.wav              $(zombonoDATADIR)/zombonogame/players/cyborg/pain25_1.wav      
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/pain25_2.wav              $(zombonoDATADIR)/zombonogame/players/cyborg/pain25_2.wav      
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/pain50_1.wav              $(zombonoDATADIR)/zombonogame/players/cyborg/pain50_1.wav      
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/pain50_2.wav              $(zombonoDATADIR)/zombonogame/players/cyborg/pain50_2.wav      
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/pain75_1.wav              $(zombonoDATADIR)/zombonogame/players/cyborg/pain75_1.wav      
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/pain75_2.wav              $(zombonoDATADIR)/zombonogame/players/cyborg/pain75_2.wav      
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/ps9000.tga                $(zombonoDATADIR)/zombonogame/players/cyborg/ps9000.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/ps9000_i.tga              $(zombonoDATADIR)/zombonogame/players/cyborg/ps9000_i.tga      
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/tris.md2                  $(zombonoDATADIR)/zombonogame/players/cyborg/tris.md2          
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/tyr574.tga                $(zombonoDATADIR)/zombonogame/players/cyborg/tyr574.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/tyr574_i.tga              $(zombonoDATADIR)/zombonogame/players/cyborg/tyr574_i.tga      
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/weapon.md2                $(zombonoDATADIR)/zombonogame/players/cyborg/weapon.md2        
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/weapon.tga                $(zombonoDATADIR)/zombonogame/players/cyborg/weapon.tga        
	cp $(ZOMBONO_DIR)/zombonogame/players/cyborg/weapon.tga.tga            $(zombonoDATADIR)/zombonogame/players/cyborg/weapon.tga.tga    
	cp -r $(ZOMBONO_DIR)/docs $(zombonoDATADIR)/docs
	cp $< $(RPMROOT)/SPECS/.
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) $<
	rm -rf $(TMPDIR)/zombono-data-$(VERSION)
	mv $(RPMROOT)/RPMS/$(NOARCH)/zombono-data-$(VERSION)-$(RPM_RELEASE).$(NOARCH).rpm archives/.

TARDIR=$(TMPDIR)/tar
tar:
	if [ ! -d archives ];then mkdir archives;fi
	$(MAKE) copyfiles COPYDIR=$(TARDIR)
	cp $(MOUNT_DIR)/linux/zombono.conf $(TARDIR)/zombono.conf
	cd $(TARDIR); sh $(MOUNT_DIR)/linux/fixperms.sh
	cd $(TARDIR); tar czvf zombono-$(VERSION_FN)-$(ARCH)-unknown-linux2.0.tar.gz *
	mv $(TARDIR)/*.tar.gz archives/.
	rm -rf $(TARDIR)


copyfiles:
	-mkdirhier $(COPYDIR)
	cp $(BUILD_RELEASE_DIR)/zombono $(COPYDIR)/zombono
	strip $(COPYDIR)/zombono
	-mkdir $(COPYDIR)/zombonogame
#	cp -R $(VWEP_DIR)/zombonogame $(COPYDIR)/.
#	for x in `find $(COPYDIR)/zombonogame -type d`; do chmod 755 $$x;done
#	for x in `find $(COPYDIR)/zombonogame -type f`; do chmod 644 $$x;done
	chmod 1777 $(COPYDIR)/zombonogame
	cp $(BUILD_RELEASE_DIR)/game$(ARCH).$(SHLIBEXT) $(COPYDIR)/zombonogame
	cp $(BUILD_RELEASE_DIR)/ref_gl.$(SHLIBEXT) $(COPYDIR)
	cp $(BUILD_RELEASE_DIR)/ref_glx.$(SHLIBEXT) $(COPYDIR)
	cp $(TDFXGL_DIR)/release$(ARCH)$(GLIBC)/lib3dfxgl.so $(COPYDIR)
	cp $(MESA_DIR)/lib/libMesaGL.so.2.6 $(COPYDIR)
	cd $(COPYDIR); ln -sf libMesaGL.so.2.6 libMesaGL.so.2
	cd $(COPYDIR); ln -sf libMesaGL.so.2 libMesaGL.so
	cd $(COPYDIR); ln -sf libMesaGL.so libGL.so
	cp $(MOUNT_DIR)/linux/README $(COPYDIR)
	cp $(MOUNT_DIR)/readme.txt $(COPYDIR)
	cp $(MOUNT_DIR)/releasenotes.txt $(COPYDIR)

zombono.spec: $(MOUNT_DIR)/linux/zombono.spec.sh
	sh $< $(VERSION_FN) $(RPM_RELEASE) $(ARCH) $(INSTALLDIR) > $@

zombono-patch.spec: $(MOUNT_DIR)/linux/zombono-patch.spec.sh
	sh $< $(VERSION_FN) $(RPM_RELEASE) $(ARCH) $(INSTALLDIR) > $@

zombono-data.spec: $(MOUNT_DIR)/linux/zombono-data.spec.sh
	sh $< $(VERSION) $(RPM_RELEASE) $(NOARCH) $(INSTALLDIR) > $@

endif


#############################################################################
# MISC
#############################################################################

clean: clean-debug clean-release

clean-debug:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean-release:
	$(MAKE) clean2 BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:
	-rm -f \
	$(ZOMBONO_OBJS) \
	$(ZombonoDedicatedServer_OBJS) \
	$(GAME_OBJS) \
	$(REF_GL_OBJS) \
	$(REF_GL_FXMESA_OBJS) \
	$(REF_GL_GLX_OBJS) \
