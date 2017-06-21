/*
*
* Advance OS 2 project - oron swissa nerya yona
*
*/

/*
TODO
  add  button

*/
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kobject.h>
#include <linux/init.h>
#include <linux/keyboard.h>
#include <linux/semaphore.h>
#include <linux/types.h>
#include <linux/timer.h>       /* Needed for timer */
#include <linux/gpio.h>       // Required for the GPIO functions
#include <asm/unistd.h>



#define DRIVER_AUTHOR "Oron Swissa and nerya yona"
#define DRIVER_DESC "Course Project - System status Led"

#define CPU_VALUE_ATTR_NAME "cpu"
#define MEMORY_VALUE_ATT_NAME "memory"
#define TO_RUN_VALUE_ATT_NAME "to_run"
#define STATUS_OBJECT_NAME "sys_status_module"

#define SECONDS_COUNT 5

/*
* leds and button config
*/
static unsigned int gpioLED27 = 27;           ///< Default GPIO for the LED is 27
static bool ledOn27 = 0;

static unsigned int gpioLED5 = 5;
static bool ledOn5 = 0;

static unsigned int gpioLED6 = 6;
static bool ledOn6 = 0;

static unsigned int gpioLED13 = 13;
static bool ledOn13 = 0;

static unsigned int gpioButton = 115;   ///< hard coding the button gpio for this example to P9_27 (GPIO115)
static unsigned int irqNumber;
static unsigned int numberPresses = 0;  //for information, store the number of Button presses

/*
*
* irq setup for Button
*
*/

static irq_handler_t irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){

	 to_show_cpu = !to_show_cpu;           //switching between cpu and memory display of the led
	 printk(KERN_INFO "GPIO_TEST: Interrupt! (button state is %d)\n", gpio_get_value(gpioButton));
   numberPresses++;                         // Global counter, will be outputted when the module is unloaded
   return (irq_handler_t) IRQ_HANDLED;      // Announce that the IRQ has been handled correctly
}
/*
* timer setup
*/
/* setup timer */
struct timer_list myTimer;
int i;


//sysfs entry declartion
static struct kobject *mykobj;
/*
* the attribute that will be shown in the sysfs
*/
static int memory =  0;
static int cpu = 0;
static int to_run = 0;
static bool to_show_cpu = 0;
static int precent = 0;

/*
* running user application from kernel space
* this should be calld from button press for now on init
* it will be called
*/
static int run_user_app(void)
{
  //struct subprocess_info *sub_info;
	///home/saw/Desktop/os2/project
  //  /home/saw/Desktop/AdvanceOS/lkp/project/a.out
	// /home/pi/Desktop/project/a.out
	char *argv[] = {"/home/saw/Desktop/project/a.out", NULL};
  static char *envp[] = {"HOME=/","TERM=linux","PATH=/sbin:/bin:/usr/sbin:/usr/bin",NULL};

  //sub_info = call_usermodehelper_setup(argv[0],argv,envp,GFP_ATOMIC,NULL,NULL,NULL);
  //if(sub_info == NULL) return -1;

  //return call_usermodehelper_exec(sub_info,UMH_WAIT_PROC);
  return call_usermodehelper(argv[0],argv,envp,UMH_WAIT_EXEC);
}

//starts the timer
static void my_set_timer(struct timer_list * mtimer)
{
	 mtimer->expires = jiffies + (HZ*SECONDS_COUNT);
    add_timer (mtimer); /* setup the timer again */
}

// timer interrupt sevice routine - executes timer action
void timerFun (unsigned long arg) {

    int tmp;
    i++;
    tmp = i;
    printk (KERN_INFO "Called timer %d times\r\n", tmp);


    if(to_show_cpu == 0)
    {
      precent = cpu;
	    printk(KERN_INFO "sys status module: show cpu");
    }
    else
    {
    	precent = memory;
	  	printk(KERN_INFO "sys status module: show memory");
    }

    //calculating how much led to turn on base on precentage
      if(precent <= 25)
      {
		printk(KERN_INFO "sys status module: gpio27 on");
        ledOn27 = false;
        //led on
        gpio_set_value(gpioLED27, ledOn27);
        //led off
        gpio_set_value(gpioLED5, true);gpio_set_value(gpioLED6, true);gpio_set_value(gpioLED13, true);
      }
      else if( precent > 25 && precent <= 50)
      {
		printk(KERN_INFO "sys status module: gpio27 and gpio5 on");
        ledOn27 = false;
        ledOn5 = false;
        //led on
        gpio_set_value(gpioLED27, ledOn27);gpio_set_value(gpioLED5, ledOn5);
        //led off
        gpio_set_value(gpioLED6, true);gpio_set_value(gpioLED13, true);
      }
      else if( precent > 50 && precent <= 75)
      {
		printk(KERN_INFO "sys status module: gpio27 & gpio5 & gpio6 on");
        ledOn27 = false;
        ledOn5 = false;
        ledOn6 = false;
        //led on
        gpio_set_value(gpioLED27, ledOn27);gpio_set_value(gpioLED5, ledOn5);gpio_set_value(gpioLED6, ledOn6);
        //led off
        gpio_set_value(gpioLED13, true);
      }
      else if( precent > 75 && precent <= 100)
      {
		   printk(KERN_INFO "sys status module: all gpio led on!");
        ledOn27 = false;
        ledOn5 = false;
        ledOn6 = false;
        ledOn13 = false;
        //led on
        gpio_set_value(gpioLED27, ledOn27);
        gpio_set_value(gpioLED5, ledOn5);
        gpio_set_value(gpioLED6, ledOn6);
        gpio_set_value(gpioLED13, ledOn13);
      }



    my_set_timer(&myTimer);

}



static struct attribute cpu_value = {
  .name = CPU_VALUE_ATTR_NAME,
  .mode = 0777,

};

static struct attribute memory_value = {
  .name = MEMORY_VALUE_ATT_NAME,
  .mode = 0777,
};

static struct attribute to_run_value = {
  .name = TO_RUN_VALUE_ATT_NAME,
  .mode = 0777,

};

static struct attribute *my_attrs[] = {
  &cpu_value,
  &memory_value,
  &to_run_value,
  NULL
};

/*
* Function show will be called when u READ from sysfs attribute
* 1 function for both attributes - print the value of the attribute
* by there name.
*/

static ssize_t show(struct kobject *kobj, struct attribute *attr, char *buff)
{
  printk(KERN_INFO "System status Module : show called by %s attribute\n", attr->name);

  if(strcmp(attr->name,CPU_VALUE_ATTR_NAME)==0)
  {
    sprintf(buff,"%d \n",cpu);
    //by convention return number of byte printed
    return sizeof(int);
  }
  if(strcmp(attr->name,MEMORY_VALUE_ATT_NAME)==0)
  {
    sprintf(buff,"%d \n",memory);
    //by convention return number of byte printed
    return sizeof(int);
  }
  if(strcmp(attr->name,TO_RUN_VALUE_ATT_NAME)==0)
  {
    sprintf(buff,"%d \n",to_run);
    //by convention return number of byte printed
    return sizeof(int);
  }

 return 0;


}

/*
* Function store will be called when u WRITE to sysfs
* 1 function for all attributes - store the value of the attribute
* by there name.
*/
static ssize_t store(struct kobject *kobj,struct attribute *attr,const char *buff,size_t len)
{
  printk(KERN_INFO "System status Module : store called by %s attribute \n" ,attr->name);

  if(strcmp(attr->name,CPU_VALUE_ATTR_NAME)==0)
  {
    int i=0;
    i=sscanf(buff,"%d \n",&cpu);
    //by convention return number of byte store
    printk(KERN_INFO "Sys status module: store to cpu %d vars\n",i);
    return sizeof(int);
  }

  if(strcmp(attr->name,MEMORY_VALUE_ATT_NAME)==0)
  {
    sscanf(buff,"%d \n",&memory);
    //by convention return number of byte store
    return sizeof(int);
  }
  if(strcmp(attr->name,TO_RUN_VALUE_ATT_NAME)==0)
  {
    sscanf(buff,"%d\n",&to_run);
    //by convention return number of byte store
    return sizeof(int);
  }

  return 0;
}


/*
* connecting the Function show and store to the sysfs entry
*/
static struct sysfs_ops my_sysfs_ops = {
  .show = show,
  .store = store,
};

static struct kobj_type my_kobj_type = {
  .sysfs_ops = &my_sysfs_ops,
  .default_attrs = my_attrs,
};

/*
* keyboard notifier
*/

struct semaphore sem;

static const char* keymap[] = { "\0", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "_BACKSPACE_", "_TAB_",
                        "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "_ENTER_", "_CTRL_", "a", "s", "d", "f",
                        "g", "h", "j", "k", "l", ";", "'", "`", "_SHIFT_", "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".",
                        "/", "_SHIFT_", "\0", "\0", " ", "_CAPSLOCK_", "_F1_", "_F2_", "_F3_", "_F4_", "_F5_", "_F6_", "_F7_",
                        "_F8_", "_F9_", "_F10_", "_NUMLOCK_", "_SCROLLLOCK_", "_HOME_", "_UP_", "_PGUP_", "-", "_LEFT_", "5",
                        "_RTARROW_", "+", "_END_", "_DOWN_", "_PGDN_", "_INS_", "_DEL_", "\0", "\0", "\0", "_F11_", "_F12_",
                        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "_ENTER_", "CTRL_", "/", "_PRTSCR_", "ALT", "\0", "_HOME_",
                        "_UP_", "_PGUP_", "_LEFT_", "_RIGHT_", "_END_", "_DOWN_", "_PGDN_", "_INSERT_", "_DEL_", "\0", "\0",
                        "\0", "\0", "\0", "\0", "\0", "_PAUSE_"};

static const char* keymapShiftActivated[] =
                        { "\0", "ESC", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "_BACKSPACE_", "_TAB_",
                        "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "_ENTER_", "_CTRL_", "A", "S", "D", "F",
                        "G", "H", "J", "K", "L", ":", "\"", "~", "_SHIFT_", "|", "Z", "X", "C", "V", "B", "N", "M", "<", ">",
                        "?", "_SHIFT_", "\0", "\0", " ", "_CAPSLOCK_", "_F1_", "_F2_", "_F3_", "_F4_", "_F5_", "_F6_", "_F7_",
                        "_F8_", "_F9_", "_F10_", "_NUMLOCK_", "_SCROLLLOCK_", "_HOME_", "_UP_", "_PGUP_", "-", "_LEFT_", "5",
                        "_RTARROW_", "+", "_END_", "_DOWN_", "_PGDN_", "_INS_", "_DEL_", "\0", "\0", "\0", "_F11_", "_F12_",
                        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "_ENTER_", "CTRL_", "/", "_PRTSCR_", "ALT", "\0", "_HOME_",
                        "_UP_", "_PGUP_", "_LEFT_", "_RIGHT_", "_END_", "_DOWN_", "_PGDN_", "_INSERT_", "_DEL_", "\0", "\0",
                        "\0", "\0", "\0", "\0", "\0", "_PAUSE_"};

static int shiftKeyDepressed = 0;

//notifier function - callback
int keylogger_notify(struct notifier_block *nblock, unsigned long code, void *_param)
{
    struct keyboard_notifier_param *param = _param;
    if (code == KBD_KEYCODE)
    {
        if( param->value==42 || param->value==54 )
        {
            //acquire lock to modify the global variable shiftKeyDepressed
            down(&sem);
            if(param->down)
                shiftKeyDepressed = 1;
            else
                shiftKeyDepressed = 0;
            up(&sem);
            return NOTIFY_OK;
        }

        if(param->down)
        {
            //acquire lock to read the global variable shiftKeyDepressed
            down(&sem);
            //if "p" or "P" presses switching to memory/cpu state
            if(shiftKeyDepressed == 0)
            {
              if(keymap[param->value] == keymap[25])
              {
                to_show_cpu = !to_show_cpu;
                printk(KERN_INFO "P has been pressed");
              }
                //printk(KERN_INFO "%s \n", keymap[param->value]);
            }
            else
            {
              if(keymapShiftActivated[param->value] == keymapShiftActivated[25])
              {
                to_show_cpu = !to_show_cpu;
                printk(KERN_INFO "P has been pressed");
              }
                //printk(KERN_INFO "%s \n", keymapShiftActivated[param->value]);
            }
            up(&sem);
        }
    }

    return NOTIFY_OK;
}

static struct notifier_block keylogger_nb =
{
    .notifier_call = keylogger_notify
};



/*
* init
*/
static int __init init_sys_status(void)
{
  int err = -1;
  int run = 42;
  //keybot notifier init
  register_keyboard_notifier(&keylogger_nb);
  printk(KERN_INFO "sys status module: Registering the keylogger module with the keyboard notifier list\n");
  sema_init(&sem, 1);



  //led and button init

  gpio_request(gpioLED27, "green");          // gpioLED27 is 27 gpio by default, request it
  gpio_direction_output(gpioLED27, 0);   // Set the gpio to be in output mode and turn on

  gpio_request(gpioLED5,"yellow");
  gpio_direction_output(gpioLED5, 0);

  gpio_request(gpioLED6,"blue");
  gpio_direction_output(gpioLED6, 0);

  gpio_request(gpioLED13,"red");
  gpio_direction_output(gpioLED13, 0);
	printk(KERN_INFO "sys status module: gpio led have been added.");

  gpio_request(gpioButton, "button");       // Set up the gpioButton
  gpio_direction_input(gpioButton);        // Set the button GPIO to be an input
  gpio_set_debounce(gpioButton, 200);      // Debounce the button with a delay of 200ms
  //gpio_export(gpioButton, false);          // Causes gpio115 to appear in /sys/class/gpio

  irqNumber = gpio_to_irq(gpioButton);
   printk(KERN_INFO "GPIO_TEST: The button is mapped to IRQ: %d\n", irqNumber);

   // This next call requests an interrupt line
   result = request_irq(irqNumber,             // The interrupt number requested
                        (irq_handler_t) irq_handler, // The pointer to the handler function below
                        IRQF_TRIGGER_RISING,   // Interrupt on rising edge (button press, not release)
                        "ebb_gpio_handler",    // Used in /proc/interrupts to identify the owner
                        NULL);                 // The *dev_id for shared interrupt lines, NULL is okay

  mykobj = kzalloc(sizeof(*mykobj),GFP_KERNEL);
  if(mykobj)
  {
    //getting error from gcc try on device or repalce to init and add
    //kobj_init(mykobj,&my_kobj_type);
    kobject_init(mykobj,&my_kobj_type);
    if(kobject_add(mykobj,NULL,"%s",STATUS_OBJECT_NAME))
    {
      err = -1;
      printk(KERN_INFO "sys status module: kobject_add() faild\n");
      kobject_put(mykobj);
      mykobj = NULL;
    }

    err = 0;

  }

  //timer init
  init_timer (&myTimer);
  myTimer.function = timerFun;
  myTimer.data = 0;

  my_set_timer(&myTimer);
  printk (KERN_INFO "sys status module:timer added. \n");

  printk(KERN_INFO "sys status module: Running run_user_app!!");
  run =  run_user_app();
  printk(KERN_INFO "sys status module: run user app return: %d",run);


  printk(KERN_INFO "sys_status_module: init finished");
  printk(KERN_INFO "sys_status_module: loaded\n");
  return 0;

}

/*
*
*/
static void __exit cleanup_sys_status(void)
{
  printk(KERN_INFO "sys_status_module: exit started");
  unregister_keyboard_notifier(&keylogger_nb);
  printk(KERN_INFO "Unregistered the keylogger module \n");

  if(mykobj)
  {
    kobject_put(mykobj);
    kfree(mykobj);
  }
  //free gpio
  gpio_set_value(gpioLED27,0);
  gpio_set_value(gpioLED5,0);
  gpio_set_value(gpioLED6,0);
  gpio_set_value(gpioLED13,0);

  gpio_free(gpioLED27);
  gpio_free(gpioLED5);
  gpio_free(gpioLED6);
  gpio_free(gpioLED13);
	printk(KERN_INFO "sys status module: removed leds");

  gpio_free(gpioButton);
  printk(KERN_INFO "sys status module: removed Button");

  //free timer
  if(!del_timer(&myTimer))
  {
	  printk(KERN_INFO "Couldn't remove timer");
  }
  else
  {
	  printk(KERN_INFO "Timer removed");
  }
  printk(KERN_INFO "sys_status_module unloaded and finished\n");


}

module_init(init_sys_status);
module_exit(cleanup_sys_status);


/*
* module license
*/
MODULE_LICENSE("GPL");

/*
*   authors and description
*/
MODULE_AUTHOR(DRIVER_AUTHOR); // who wrote this module
MODULE_DESCRIPTION(DRIVER_DESC);//what does this module do

/*
*This module uses /dev/testdevice. The MODULE_SUPPORTED_DEVICE macro might
*be used in the futur to help automate configuration of modules, but is
*currently unsued other than for documentation
*/

MODULE_SUPPORTED_DEVICE("Rasspebry pi 3");
