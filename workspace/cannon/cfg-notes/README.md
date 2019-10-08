# CFG notes

## Domain

`DOMAIN(id){args}`

where `args` is any of `CRE_TSK, DEF_TEX, SAC_TSK`, etc.

Differentiates from `KERNEL_DOMAIN`, presumably relating to access permissions

## Kernel Domain

`KERNEL_DOMAIN{args}`

where `args` is any of `CRE_TSK, DEF_TEX, SAC_TSK`, etc.

## Create task

https://www.tron.org/wp-content/themes/dp-magjam/pdf/specifications/en_US/WG024-S001-04.03.00_en.pdf page 85

`CRE_TSK(taskid,{task_attr, ext_info, task, init_prio, stack_size, stack})`

```
CRE_TSK( 
        TASK1, 
        {
            TA_NULL, 
            1, 
            task, 
            priority, 
            stack size, 
            NULL
        }
    )
```

Creates a task with the properties

- `taskid`, the task identifier
- `task_attr`, attribute `TA_NULL` or `TA_ACT` - determines whether or not the task should run at the start
- `ext_info`, extended information integer, usually the same number as the one in the identifier (0 indexed, but main is the first)
- `task`, function name
- `init_prio`, the scheduler priority of the given task
- `stack_size`, the stack size for the process in bytes
- `stack` optional address of the stack - usually `NULL`

## Task exception handling routine

https://www.tron.org/wp-content/themes/dp-magjam/pdf/specifications/en_US/WG024-S001-04.03.00_en.pdf page 121

`DEF_TEX(taskid,{task_attr, task})`

```
DEF_TEX( 
    TASK1, 
    {
        TA_NULL, 
        tex_routine 
    }
)
```

Creates a task exception handling routine(**TEX**) with the properties

- `taskid`, identifier of the task the TEX belongs to
- `task_attr`, attribute `TA_NULL` or `TA_ACT` - determines whether or not the TEX should run at the start
- `task`, function name of the TEX

## SAC_TSK

Mostly presumed based on http://tool-support.renesas.com/autoupdate/support/onlinehelp/csp/V7.00.00/CS+.chm/Coding-RI600PX.chm/Output/srv_sac_mem.html and https://github.com/ev3rt-git/ev3rt-hrp2/blob/master/doc/user.txt

`SAC_TSK(taskid,{acptn, acptn, acptn, acptn})`

    SAC_TSK( 
        TASK1, 
        {
            TACP(DOM1)|TACP(DOM2), 
            TACP(DOM1), 
            TACP(DOM1),
            TACP(DOM1)
        }
    )
Sets access permissions of a task with the properties

- `taskid`, identifier of the task that should receive permissions
- `acptn` access permission pattern - denoted by `TACP(DOMAIN)` in ev3rt, I assume the access permissions of the task is set to that of the domain

## AID_TSK

Extracted from obscure japanese beamer presentation:

> Object pool pattern - statically defined maximum amount of tasks that can be dynamically allocated

`AID_TSK(int)`

```
AID_TSK(3);
```

Sets the maximum amount of tasks that can be dynamically allocated to `int`

## Cyclic handler

https://www.tron.org/wp-content/themes/dp-magjam/pdf/specifications/en_US/WG024-S001-04.03.00_en.pdf page 247

`CRE_CYC(cycid,{cyc_attr, ext_info, cyc_handler, cyc_time, cyc_phase})`

```
CRE_CYC( 
	CYCHDR1, 
    {
        TA_NULL, 
        0, 
        cyclic_handler, 
        2000, 
        0 
    }
)
```

Creates a cyclic handler(**CYC**) with the properties

- `cycid` the identifier of the cyclic handler
- `cyc_attr`, attribute `TA_NULL` or `TA_ACT` - determines whether or not the CYC should run at the start
- `ext_info`, extended information integer, usually the same number as the one in the identifier minus one (0 indexed)
- `cyc_handler` function name of the CYC
- `cyc_time` activation cycle of the CYC
- `cyc_phase` activation phase of the CYC

## Alarm handler

https://www.tron.org/wp-content/themes/dp-magjam/pdf/specifications/en_US/WG024-S001-04.03.00_en.pdf page 256

`CRE_ALM(almid,{alm_attr, ext_info, alm_handler})`

```
CRE_ALM( 
    ALMHDR1, 
    {
        TA_NULL,
        0, 
        alarm_handler 
    }
)
```

Creates an alarm handler(**ALM**) with the properties

- `almid` the identifier of the alarm handler
- `alm_attr`, attribute `TA_NULL` or `TA_ACT` - determines whether or not the ALM should run at the start
- `ext_info`, extended information integer, usually the same number as the one in the identifier minus one (0 indexed)
- `alm_handler`, the function name of the alarm handler

## Overrun handler

https://www.tron.org/wp-content/themes/dp-magjam/pdf/specifications/en_US/WG024-S001-04.03.00_en.pdf page 264

`DEF_OVR({ovr_attr, ovr_handler})`

```
#ifdef TOPPERS_SUPPORT_OVRHDR // if toppers support for overrun handler is defined
DEF_OVR({
    TA_NULL, // dont run at start
    overrun_handler // overrun handler function name
})
#endif /* TOPPERS_SUPPORT_OVRHDR */
```

Creates an overrun handler(**OVR**) with the properties

- `ovr_attr`, attribute `TA_NULL` or `TA_ACT` - determines whether or not the OVR should run at the start
- `ovr_handler`, function name of the overrun handler

## CPU exception handler

https://www.tron.org/wp-content/themes/dp-magjam/pdf/specifications/en_US/WG024-S001-04.03.00_en.pdf page 302

`DEF_EXC(exc_id,{exc_attr, exc_handler})`

```
#ifdef CPUEXC1 // if cpu exception handler 1 is defined?
DEF_EXC( 
    CPUEXC1, 
    {
        TA_NULL,
        cpuexc_handler 
    }
)
#endif /* CPUEXC1 */
```

Creates a CPU exception handler(**EXC**) with the properties

- `exc_id` the identifier of the EXC
- `exc_attr`, attribute `TA_NULL` or `TA_ACT` - determines whether or not the EXC should run at the start
- `cpuexc_handler`, function name of the EXC

## SAC_SYS

Mostly presumed based on http://tool-support.renesas.com/autoupdate/support/onlinehelp/csp/V7.00.00/CS+.chm/Coding-RI600PX.chm/Output/srv_sac_mem.html and https://github.com/ev3rt-git/ev3rt-hrp2/blob/master/doc/user.txt

`SAC_SYS({acptn, acptn, acptn, acptn})`

```
 SAC_SYS({ 
     TACP(DOM1)|TACP(DOM2), 
     TACP(DOM2), 
     TACP_KERNEL, 
     TACP_KERNEL 
 })
```

Sets access permissions of the system with the properties

- `acptn` access permission pattern
  - first two are related to regular domain, last two are related to kernel domain

## DEF_KMM

Mostly presumed based on https://github.com/ev3rt-git/ev3rt-hrp2/blob/master/doc/user.txt

`DEF_KMM({kmm_size, stack})` // SIZE kmmsz, STK_T kmm

```
DEF_KMM({ // define kernel memory, probably
    KMM_SIZE, // kernel memory, probably, defined in header as stack size * 16
    NULL // 'SIZE kmmsz, STK_T *kmm' - stack type, probably
})
```

Sets kernel memory(**KMM**) with the properties

- `kmm_size`, size of the kernel memory - in sample1.h, it is defined as the static stack size of 4096 bytes, times 16
- `stack`, probably a stack object - docs says `STK_T`

## Interrupt handler

https://www.tron.org/wp-content/themes/dp-magjam/pdf/specifications/en_US/WG024-S001-04.03.00_en.pdf page 285

`DEF_INH(inhid,{inh_attr, inh})`

```
DEF_INH(
	INH1,
	{
		TA_NULL,
		interrupt_handler
	}
)
```

Creates an interrupt handler (**INH**) with the properties

- `inhid`, the identifier of the INH
- `inh_attr`, attribute `TA_NULL` or `TA_ACT` - determines whether or not the INH should run at the start
- `interrupt_handler`, the function name of the INH

## Others

`TMIN_TPRI`, task minimum priority

`TMAX_TPRI`, task maximum priority

# Resources

[EV3RT Git Docs](https://github.com/ev3rt-git/ev3rt-hrp2/blob/master/doc/user.txt) line 695 onwards

[sac_mem at Renesas](http://tool-support.renesas.com/autoupdate/support/onlinehelp/csp/V7.00.00/CS+.chm/Coding-RI600PX.chm/Output/srv_sac_mem.html) TACP / ACPTN var == access permission pattern

[Obscure Japanese Beamer Presentation on Toppers](https://www.toppers.jp/docs/etrobo15/ev3rt_seminar_04.pdf)

[µITRON Docs](https://www.tron.org/wp-content/themes/dp-magjam/pdf/specifications/en_US/WG024-S001-04.03.00_en.pdf)