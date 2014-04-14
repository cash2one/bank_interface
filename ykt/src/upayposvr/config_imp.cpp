/**
 * config_imp.cpp
 *
 *  Created on: 2011-4-18
 *      Author: WenJian
 */

#include "config_imp.h"

struct config_imp config_obj;

bool init_config(const string pathname)
{
	string path = pathname;

	try
	{
		ConfigFile config(pathname);
		try
		{
			 config.readInto(config_obj.connectinfo, "connectinfo");
			 config.readInto(config_obj.log_conf, "logconf");
			 config.readInto(config_obj.server_ip,"netsvr_ip");
			 config.readInto(config_obj.server_port,"netsvr_port");
		}
		catch(ConfigFile::key_not_found& ex)
		{
			cout << "read "<<ex.key<<" faild" << endl;
			return false;
		}
		return true;
	}
	catch(ConfigFile::file_not_found& ex)
	{
		cout << "read file "<<ex.filename<<" faild " <<endl;
		return false;
	}
	return true;
}
