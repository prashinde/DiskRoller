#ifndef LOGGER_H
#define LOGGER_H

#define INFO	KERN_INFO
#define ERROR	KERN_ERR
#define CRIT	KERN_CRIT
#define ALERT	KERN_ALERT

#define LOG_MSG(LEVEL, __FORMAT__, ...) printk(LEVEL "Function:%s:"__FORMAT__, __FUNCTION__, ## __VA_ARGS__) 

#endif
