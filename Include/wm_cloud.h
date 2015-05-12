#ifndef __WM_CLOUD_H__
#define __WM_CLOUD_H__

enum cloud_opt_type
{
	UPLOAD_REQ,
	UPLOAD_RESP,
	CONTROL_REQ,
	CONTROL_RESP,
	SNAPSHOT_REQ,
	SNAPSHOT_RESP
};
typedef struct _CloudData
{
	enum cloud_opt_type opt;
	int result;
	char* control_resp;
	char** names;
	char** values;
	int cnt;
	void * arg;
} CloudData;


typedef int (*cloud_callback)(CloudData* data);
int tls_cloud_init(void * arg);
void tls_cloud_finish(u8 block);
int tls_cloud_start_config(void);
int tls_cloud_upload_data(CloudData* data);
int tls_cloud_set_callback(cloud_callback callback);

#endif

