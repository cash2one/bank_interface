/**
 * config_imp.h
 *
 *  Created on: 2010-4-18
 *      Author: WenJian
 */

#ifndef CONFIG_IMP_H_
#define CONFIG_IMP_H_

#include <string>
//#include "logger_imp.h"
#include "ConfigFile.h"

using namespace std;

struct config_imp
{
   string log_conf; 	//Log配置
   string connectinfo;	//数据库连接串
   std::size_t service_code; //业务功能号
   std::size_t internal_id; // 工作组编号
   string server_ip; //netsvr ip
   std::size_t server_port; // netsvr port
};

extern config_imp config_obj;
bool init_config(const string pathname);

#endif /* CONFIG_IMP_H_ */
