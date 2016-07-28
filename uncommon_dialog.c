// A drop-in replacement for sceMsgDialog because xyz is too lazy to figure out why actual dialogs don't work

// NB not actually drop-in, you need to change your code a bit: change order of vita2d_common_dialog_update/vita2d_end_drawing
// before: vita2d_end_drawing(); vita2d_common_dialog_update();
// after: vita2d_common_dialog_update(); vita2d_end_drawing();

// Also there's absolutely zero error checking, and this only was ever tested with APIs vitashell uses. Enjoy!

#include <stdint.h>

#include <psp2/ctrl.h>
#include <psp2/types.h>
#include <psp2/message_dialog.h>

#include <vita2d.h>

typedef struct dialog {
	int status;
	int mode;
	int buttonType;
	int buttonId;
	char msg[4096];
	int progress;
} dialog;

static dialog g_dialog;

enum {
	SCREEN_WIDTH = 960,
	SCREEN_HEIGHT = 544,
	DIALOG_WIDTH = 700,
	DIALOG_HEIGHT = 400,
	DIALOG_X = (SCREEN_WIDTH - DIALOG_WIDTH) / 2,
	DIALOG_Y = (SCREEN_HEIGHT - DIALOG_HEIGHT) / 2,
	DIALOG_PADDING = 30,
	DIALOG_CONTENT_X = DIALOG_X + DIALOG_PADDING,
	DIALOG_CONTENT_Y = DIALOG_Y + DIALOG_PADDING,
	DIALOG_CONTENT_WIDTH = DIALOG_WIDTH - DIALOG_PADDING * 2,
	DIALOG_CONTENT_HEIGHT = DIALOG_HEIGHT - DIALOG_PADDING * 2,
};

int sceMsgDialogProgressBarSetValue(SceMsgDialogProgressBarTarget target, SceUInt32 rate) {
	g_dialog.progress = rate;
	return 0;
}

int sceMsgDialogInit(const SceMsgDialogParam *param) {
	sceClibPrintf("sceMsgDialogInit type %d\n", param->mode);
	memset(&g_dialog, 0, sizeof(g_dialog));
	switch (param->mode) {
		case SCE_MSG_DIALOG_MODE_USER_MSG: {
			strncpy(g_dialog.msg, param->userMsgParam->msg, sizeof(g_dialog.msg) - 1);
			g_dialog.buttonType = param->userMsgParam->buttonType;
			break;
		}
		case SCE_MSG_DIALOG_MODE_PROGRESS_BAR: {
			strncpy(g_dialog.msg, param->progBarParam->msg, sizeof(g_dialog.msg) - 1);
			g_dialog.buttonType = -1;
			break;
		}
		default:
			return -1;
	}

	g_dialog.mode = param->mode;
	g_dialog.status = SCE_COMMON_DIALOG_STATUS_RUNNING;

	return 0;
}

SceCommonDialogStatus sceMsgDialogGetStatus(void) {
	return g_dialog.status;
}

int sceMsgDialogClose(void) {
	sceClibPrintf("sceMsgDialogClose\n");
	g_dialog.status = SCE_COMMON_DIALOG_STATUS_FINISHED;
	return 0;
}

int sceMsgDialogTerm(void) {
	sceClibPrintf("sceMsgDialogTerm\n");
	g_dialog.status = SCE_COMMON_DIALOG_STATUS_NONE;
	return 0;
}

int sceMsgDialogGetResult(SceMsgDialogResult *result) {
	sceClibPrintf("sceMsgDialogGetResult\n");
	result->buttonId = g_dialog.buttonId;
	return 0;
}

void update_buttons() {
	static uint32_t saved;
	uint32_t prev, new_btns;
	SceCtrlData ctrl;

	prev = saved;
	sceCtrlReadBufferPositive(0, &ctrl, 1);
	new_btns = ctrl.buttons & (ctrl.buttons ^ prev);
	saved = ctrl.buttons;

	if (g_dialog.status != SCE_COMMON_DIALOG_STATUS_RUNNING)
		return;

	switch (g_dialog.buttonType) {
		case SCE_MSG_DIALOG_BUTTON_TYPE_OK:
		case SCE_MSG_DIALOG_BUTTON_TYPE_CANCEL: {
			if (new_btns & SCE_CTRL_CIRCLE)
				g_dialog.status = SCE_COMMON_DIALOG_STATUS_FINISHED;
			break;
		}
		case SCE_MSG_DIALOG_BUTTON_TYPE_YESNO: {
			if (new_btns & (SCE_CTRL_CIRCLE | SCE_CTRL_CROSS)) {
				g_dialog.status = SCE_COMMON_DIALOG_STATUS_FINISHED;
				g_dialog.buttonId = (new_btns & SCE_CTRL_CIRCLE) ? SCE_MSG_DIALOG_BUTTON_ID_NO : SCE_MSG_DIALOG_BUTTON_ID_YES;
			}
			break;
		}
	}
}

extern vita2d_pgf *font; // defined by VitaShell
int sceCommonDialogUpdate(const SceCommonDialogUpdateParam *updateParam) {
	update_buttons();

	if (g_dialog.status != SCE_COMMON_DIALOG_STATUS_RUNNING)
		return 0;

	// backdrop
	vita2d_draw_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x80FF0000);
	// shadow
	vita2d_draw_rectangle(DIALOG_X+10, DIALOG_Y+10, DIALOG_WIDTH, DIALOG_HEIGHT, 0x70000000);
	// border
	vita2d_draw_rectangle(DIALOG_X-1, DIALOG_Y-1, DIALOG_WIDTH+2, DIALOG_HEIGHT+2, 0xFFFFFFFF);
	// dialog background
	vita2d_draw_rectangle(DIALOG_X, DIALOG_Y, DIALOG_WIDTH, DIALOG_HEIGHT, 0xFF333333);
	// text
	vita2d_pgf_draw_text(font, DIALOG_CONTENT_X, DIALOG_CONTENT_Y + 20, 0xFFFFFFFF, 1, g_dialog.msg);

	const char *button_info = "";
	switch (g_dialog.buttonType) {
	case SCE_MSG_DIALOG_BUTTON_TYPE_OK:
	case SCE_MSG_DIALOG_BUTTON_TYPE_CANCEL:
		button_info = "CIRCLE = back";
		break;
	case SCE_MSG_DIALOG_BUTTON_TYPE_YESNO:
		button_info = "CROSS = yes / CIRCLE = no";
		break;
	}
	vita2d_pgf_draw_text(font, DIALOG_CONTENT_X, DIALOG_CONTENT_Y + DIALOG_CONTENT_HEIGHT, 0xFFFFFFFF, 1, button_info);

	if (g_dialog.mode == SCE_MSG_DIALOG_MODE_PROGRESS_BAR) {
		float progress_bar_x = DIALOG_CONTENT_X;
		float progress_bar_y = DIALOG_CONTENT_Y + DIALOG_CONTENT_HEIGHT/2;
		float progress_bar_w = DIALOG_CONTENT_WIDTH;
		float progress_bar_h = 10;
		// progress bar border
		vita2d_draw_rectangle(progress_bar_x-1, progress_bar_y-1, progress_bar_w+2, progress_bar_h+2, 0xFFCCCCCC);
		// progress bar background
		vita2d_draw_rectangle(progress_bar_x, progress_bar_y, progress_bar_w, progress_bar_h, 0xFF111111);
		// progress bar
		vita2d_draw_rectangle(progress_bar_x, progress_bar_y, progress_bar_w * g_dialog.progress / 100.0, progress_bar_h, 0xFFFFFFFF);
	}

	return 0;
}
