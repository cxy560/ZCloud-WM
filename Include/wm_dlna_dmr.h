#ifndef __WM__DLNA_DMR_H_
#define __WM__DLNA_DMR_H_

enum dmr_control_type
{
	CONTROL_GET = 0,
	CONTROL_SET
};
/*************************************************************************************************************************
* Description: This Function prototype for action callback functions. 
*
* Arguments  : None.
*
* Returns    :  None
*************************************************************************************************************************/
typedef void (*dmr_action_callback)(void);
/*************************************************************************************************************************
* Description: This Function prototype for control callback functions.
*
* Arguments  : type    get or set control.
*
*                    channel   the channel to control
*
*                    value      the value of getting or wanting to set
*
* Returns    :  None
*************************************************************************************************************************/
typedef void (*dmr_control_callback)(enum dmr_control_type type, const char* channel, int* value);
/*************************************************************************************************************************
* Description: This Function prototype for getting the playing progress.
*
* Arguments  : totlen    The total length of the playing media.
*
* Returns    :  the progress(0.0 ~ 1.0) of having been played.
*************************************************************************************************************************/
typedef float (*dmr_play_progress_callback)(int totlen);
/*************************************************************************************************************************
* Description: This Function prototype for downloading media data finished.
*
* Arguments  : buf     The downloaded data this time.
*
*                    datalen            The length of buffer data.
*
* Returns    :  None
*************************************************************************************************************************/
typedef void (*dmr_download_finish_callback)(char * buf, int datalen);

/*************************************************************************** 
* Function: tls_dmr_set_play_callback 
* Description: Set play callback which would be called while starting to play.
* 
* Input: callback:      the play callback
* 
* Output: None
* 
* Return: None
* 
* Date : 2014-11-06  
****************************************************************************/ 
void tls_dmr_set_play_callback(dmr_action_callback callback);
/*************************************************************************** 
* Function: tls_dmr_set_stop_callback 
* Description: Set stop callback which would be called while to stop.
* 
* Input: callback:      the stop callback
* 
* Output: None
* 
* Return: None
* 
* Date : 2014-11-06  
****************************************************************************/ 
void tls_dmr_set_stop_callback(dmr_action_callback callback);
/*************************************************************************** 
* Function: tls_dmr_set_seek_callback 
* Description: Set seek callback which would be called while to seek.
* 
* Input: callback:      the seek callback
* 
* Output: None
* 
* Return: None
* 
* Date : 2014-11-06  
****************************************************************************/ 
void tls_dmr_set_seek_callback(dmr_action_callback callback);
/*************************************************************************** 
* Function: tls_dmr_set_pause_callback 
* Description: Set pause callback which would be called while to pause.
* 
* Input: callback:      the pause callback
* 
* Output: None
* 
* Return: None
* 
* Date : 2014-11-06  
****************************************************************************/ 
void tls_dmr_set_pause_callback(dmr_action_callback callback);
/*************************************************************************** 
* Function: tls_dmr_set_play_progress_callback 
* Description: Set play progress callback which would be called while to get play progress.
* 
* Input: callback:      the play progress callback
* 
* Output: None
* 
* Return: None
* 
* Date : 2014-11-06  
****************************************************************************/ 
void tls_dmr_set_play_progress_callback(dmr_play_progress_callback callback);
/*************************************************************************** 
* Function: tls_dmr_set_mute_callback 
* Description: Set mute control callback which would be called while to get or set mute control.
* 
* Input: callback:      the mute control callback
* 
* Output: None
* 
* Return: None
* 
* Date : 2014-11-06  
****************************************************************************/ 
void tls_dmr_set_mute_callback(dmr_control_callback callback);
/*************************************************************************** 
* Function: tls_dmr_set_volume_callback 
* Description: Set volume control callback which would be called while to get or set volume control.
* 
* Input: callback:      the volume control callback
* 
* Output: None
* 
* Return: None
* 
* Date : 2014-11-06  
****************************************************************************/ 
void tls_dmr_set_volume_callback(dmr_control_callback callback);
/*************************************************************************** 
* Function: tls_dmr_set_volumedb_callback 
* Description: Set volumedb control callback which would be called while to get or set volumedb control.
* 
* Input: callback:      the volumedb control callback
* 
* Output: None
* 
* Return: None
* 
* Date : 2014-11-06  
****************************************************************************/ 
void tls_dmr_set_volumedb_callback(dmr_control_callback callback);
/*************************************************************************** 
* Function: tls_dmr_set_loudness_callback 
* Description: Set loudness control callback which would be called while to get or set loudness control.
* 
* Input: callback:      the loudness control callback
* 
* Output: None
* 
* Return: None
* 
* Date : 2014-11-06  
****************************************************************************/ 
void tls_dmr_set_loudness_callback(dmr_control_callback callback);
/*************************************************************************** 
* Function: tls_dmr_init 
* Description: Initialize the media render.
* 
* Input: friendly_name:      The friendly name of the media render.
*
*           uuid:                    The uuid of the media render.
* 
* Output: None
* 
* Return: 0: Succeed; minus: Failed. 
* 
* Date : 2014-11-06  
****************************************************************************/ 
int tls_dmr_init(const char *friendly_name, const char *uuid);
/*************************************************************************** 
* Function: tls_dmr_download_data 
* Description: This function for triggering to download data from media server, the download_callback
*                    would be called when downloading finished.
* 
* Input: dest:       The destination address to download data to.
*
*           downsize:      The download size of this time.
*
*           download_callback:      The download finished callback.
* 
* Output: None
* 
* Return: 0: Succeed; minus: Failed. 
* 
* Date : 2014-11-06  
****************************************************************************/ 
int tls_dmr_download_data(char * dest, int downsize, dmr_download_finish_callback download_callback);


#endif //__WM__DLNA_DMR_H_

