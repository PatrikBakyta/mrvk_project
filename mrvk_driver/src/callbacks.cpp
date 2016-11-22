/*
 * callbacks.cpp
 *
 *  Created on: 26.10.2016
 *      Author: michal
 */

#include <mrvk_driver/callbacks.h>

MrvkCallbacks::MrvkCallbacks(CommunicationInterface *interface) {

	this->interface = interface;

	ros::NodeHandle n;

	//initSS = n.advertiseService("init", &MrvkCallbacks::initCallback, this);
	//resetFlagsSS = n.advertiseService("reset_flags", &MrvkCallbacks::resetFlagsCallback, this);
	shutdownSS = n.advertiseService("shutdown", &MrvkCallbacks::shutdownCallback, this);
	resetCentralStopSS = n.advertiseService("reset_central_stop", &MrvkCallbacks::resetCentralStopCallback, this);
	resetBatterySS = n.advertiseService("reset_Q_batery", &MrvkCallbacks::resetBatteryCallback, this);
	//stopSS = n.advertiseService("set_global_stop", &MrvkCallbacks::stopCallback, this);
	setArmVoltageSS = n.advertiseService("set_arm_voltage", &MrvkCallbacks::setArmVoltageCallback, this);
	setCameraSourceSS = n.advertiseService("camera_source", &MrvkCallbacks::setCameraSourceCallback, this);
	setPowerManagmentSS = n.advertiseService("write_main_board_settings", &MrvkCallbacks::setPowerManagmentCallback, this);
	///setMotorParametersSS = n.advertiseService("write_motor_settings", &MrvkCallbacks::setMotorParametersCallback, this);

}

/*void MrvkCallbacks::resetCallbacksFlags() {

	inter = 0;
}*/

/*
bool MrvkCallbacks::resetFlagsCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res) {

		//resetFlags = true;
		//newCallback = true;

		res.success = true;
		res.message = "flags are reset";
		return true;
	}*/

	//todo overit funkcnost
	bool MrvkCallbacks::resetBatteryCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res){

		pthread_mutex_t notify_mutex;
		pthread_mutex_init(&notify_mutex,NULL);
		pthread_mutex_lock(&notify_mutex);
		interface->registerMutex(&notify_mutex,CommunicationInterface::MAIN_BOARD);
		interface->getMainBoard()->resetBatery();
		pthread_mutex_lock(&notify_mutex);
		pthread_mutex_destroy(&notify_mutex);
		res.success = interface->getCallbackResult(CommunicationInterface::MAIN_BOARD);
		if (res.success)
			res.message = "battery is reset";
		else res.message = "battery is not reset";
		return true;
	}

	//TODO uplne prerobit reset central stop Miso - neviem ako si predstavujes prerobit reset central stop
	bool MrvkCallbacks::resetCentralStopCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res){
/*
		res.success = interface->resetCentralStop();
		if (res.success)
			res.message = "central stop is successful reset";
		else res.message = "Central stop was not reset";

		ROS_ERROR("central stop callback");
		return true;*/
		pthread_mutex_t notify_mutex;
		pthread_mutex_init(&notify_mutex,NULL);
		pthread_mutex_lock(&notify_mutex);
		uint8_t MCB_command[21];
		uint8_t MB_command[21];
		uint8_t request[5];

		interface->registerMutex(&notify_mutex,CommunicationInterface::LEFT_MOTOR);
		interface->getMotorControlBoardLeft()->setErrFlags(true,true);
		pthread_mutex_lock(&notify_mutex);
		if(!interface->getCallbackResult(CommunicationInterface::LEFT_MOTOR)){
			ROS_ERROR("FAILED to reset left motor flags");
			return false;
		}

		interface->registerMutex(&notify_mutex,CommunicationInterface::RIGHT_MOTOR);
		interface->getMotorControlBoardRight()->setErrFlags(true,true);
		pthread_mutex_lock(&notify_mutex);
		if(!interface->getCallbackResult(CommunicationInterface::RIGHT_MOTOR)){
			ROS_ERROR("FAILED to reset right motor flags");
			return false;
		}

		interface->registerMutex(&notify_mutex,CommunicationInterface::MAIN_BOARD);
		interface->getMainBoard()->setCentralStop(false);
		pthread_mutex_lock(&notify_mutex);
		if(!interface->getCallbackResult(CommunicationInterface::MAIN_BOARD)){
			ROS_ERROR("FAILED to reset central stop");
			return false;
		}

		usleep(500000);
		//if(interface->get)
		return true;
	}


	/*bool MrvkCallbacks::stopCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res)
	{
		//stop = true;
		////newCallback = true;
		 //stop();
		 res.success = true;
		 return true;
	}*/

	//100% funkcny servis
	bool MrvkCallbacks::setArmVoltageCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res){

		bool success = false;
		static bool state = false;
		ros::NodeHandle n;
		pthread_mutex_t notify_mutex;
		if(pthread_mutex_init(&notify_mutex,NULL)){
			ROS_ERROR("nezbehol init mutexu");
			return false;
		}
		pthread_mutex_lock(&notify_mutex);
		interface->registerMutex(&notify_mutex,CommunicationInterface::MAIN_BOARD);
		if (state){
			interface->getMainBoard()->setArmPower(false);
			pthread_mutex_lock(&notify_mutex);
			pthread_mutex_destroy(&notify_mutex);

			res.success = interface->getCallbackResult(CommunicationInterface::MAIN_BOARD);
			if (res.success){
				res.message = "Napajanie ramena vypnute";
				state = false;
				n.setParam("arm5V", false);
				n.setParam("arm12V", false);
			}
			else res.message = "error";
		}
		else{
			interface->getMainBoard()->setArmPower(true);
			ROS_ERROR("cakam na mutexe");
			pthread_mutex_lock(&notify_mutex);
			ROS_ERROR("po mutexe");
			pthread_mutex_destroy(&notify_mutex);

			res.success = interface->getCallbackResult(CommunicationInterface::MAIN_BOARD);
			if (res.success){
				res.message = "Napajanie ramena zapnute";
				state = true;
				n.setParam("arm5V", true);
				n.setParam("arm12V", true);
			}
			else res.message = "error";
		}
		return true;
	}
	//todo overit funkcnost
	bool MrvkCallbacks::setCameraSourceCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res) {

		/*mb->commandID = CommunicationInterface::PARTIAL_COMMAND;
		bool success = false;

		if (mb->videoGrabber){
			mb->videoGrabber = false;

			receiveAnswer(&success);
			res.success = success;
			if (res.success)
				res.message =  "Kamera na video transmitter";
			else res.message = "error";


		} else{
			mb->videoGrabber = true;
			receiveAnswer(&success);
			res.success = success;
			if (res.success)
				res.message =  "Kamera na grabbery";
			else res.message = "error";
		}*/

		pthread_mutex_t notify_mutex;
		pthread_mutex_init(&notify_mutex,NULL);
		pthread_mutex_lock(&notify_mutex);
		interface->registerMutex(&notify_mutex,CommunicationInterface::MAIN_BOARD);
		bool ret = interface->getMainBoard()->switchVideo();
		pthread_mutex_lock(&notify_mutex);
		pthread_mutex_destroy(&notify_mutex);
		if(!interface->getCallbackResult(CommunicationInterface::MAIN_BOARD)){
			res.success = false;
			interface->getMainBoard()->switchVideo(); // zmena spet
			res.message = "Video switch FAILED!!!!!";
		}
		//TODO dorobit vypis
		if(ret)
			res.message = "neviem kedy je zapnuta a kedy vypnuta";
		else
			res.message = "neviem kedy je zapnuta a kedy vypnuta";
		return true;
	}

	//100% funkcny servis
	bool MrvkCallbacks::setPowerManagmentCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res){

		pthread_mutex_t notify_mutex;
		pthread_mutex_init(&notify_mutex,NULL);
		pthread_mutex_lock(&notify_mutex);

		bool success = false;
		SET_MAIN_BOARD config;
		getMbFromParam(&config);

		interface->registerMutex(&notify_mutex,CommunicationInterface::MAIN_BOARD);
		interface->getMainBoard()->setParamatersMB(&config);

		pthread_mutex_lock(&notify_mutex);
		pthread_mutex_destroy(&notify_mutex);
		res.success = interface->getCallbackResult(CommunicationInterface::MAIN_BOARD);
		return true;
	}

	/*bool MrvkCallbacks::setMotorParametersCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res){

		//setMotorParameters = true;
		//newCallback = true;

		res.success = true;
		return true;
	}*/

/*	bool MrvkCallbacks::initCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res){

		if (init())
			res.success = true;
		else res.success = false;

		return true;
	}*/

	bool MrvkCallbacks::shutdownCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res){

		interface->close();
		ros::shutdown();
		return true;
	}

	void MrvkCallbacks::getMbFromParam(SET_MAIN_BOARD *config){

		ros::NodeHandle n;
		n.param<bool>("MCBsSB_5V", config->MCBsSB_5V, true);
		n.param<bool>("MCBs_12V", config->MCBs_12V, true);
		n.param<bool>("wifi", config->wifi, true);
		n.param<bool>("video_transmitter", config->videoTransmitter, false);
		n.param<bool>("laser_scanner", config->laser, true);
		n.param<bool>("gps", config->GPS, false);
		n.param<bool>("pc2", config->PC2, true);
		n.param<bool>("camera", config->kamera, false);
		n.param<bool>("arm5V", config->ARM_5V, false);
		n.param<bool>("arm12V", config->ARM_12V, false);
		bool video;
		n.param<bool>("video", video, true);
		config->video1 = video;
		config->video2 = !video;

		int pko, pkk, iko, ikk;

		n.param<int>("kamera_PID_pko", pko, 10);
		n.param<int>("kamera_PID_pkk", pkk, 10);
		n.param<int>("kamera_PID_iko", iko, 40);
		n.param<int>("kamera_PID_ikk", ikk, 80);

		config->pko = pko;
		config->pkk = pkk;
		config->iko = iko;
		config->ikk = ikk;
	}

	void MrvkCallbacks::getMotorParametersFromParam(REGULATOR_MOTOR *reg, bool *regulation_type){

		ros::NodeHandle n;

		int ph, pl,ih, il;
		n.param<int>("motor_PID_ph",ph, 0);
		n.param<int>("motor_PID_pl",pl, 10);
		n.param<int>("motor_PID_ih",ih, 0);
		n.param<int>("motor_PID_il",il, 15);

		reg->PH = ph;
		reg->PL = pl;
		reg->IH = ih;
		reg->IL = il;

		//bool regulation_type;
		n.param<bool>("typ_regulacie_motora", *regulation_type, false);

	}