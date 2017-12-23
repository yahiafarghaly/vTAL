# vTAL

vTAL (Virtual Timer Abstraction Layer) aims to provide a standard interface for using multiple of logic timers simultaneously while a one physical timer is presented on the target system.

vTAL presents standard APIs call for application layer developing and low level calls of the physical timer so it can be ported on any system and get the benefit of using so many virtual timers as you want while the system has only one physical hardware timer.

The project is mainly aimed to be used in an embedded software environment but also, it can be used with Linux/Windows applications.

The project consists of two main parts which are standarized:

- vTAL (Virtual Timer Abstraction Layer).
- HTAL (Hardware Timer Abstraction Layer).

## vTAL description

This is the part which involves with the application developing. It is very abstracted so you don't feel the details of the hardware timer.

## HTAL description

This layer contains standard APIs which expected to do a certain functionlity so vTAL algorithm for handling too many virtual timers can work properly.

## What makes it special

- To use vTAL, you only need to write the implementation of HTAL APIs for the target system. And write it for a single shot timer case.
- The implementation of vTAL doesn't depend on memory allocation on heap. Only array in the stack and the size is configured from **vTALConfig.h**
- Nice interface of anything exist around.
- Written only in C and compliant to ISO 9899:1990 as possible.

## Documentations

- [vTAL Interfaces](Docs/vTAL.md)
- [HTAL Interfaces](Docs/HTAL.md)

## It is ported to
- Any OS based on Linux Kernel(> v2.6)
- Stellaris LM4F120XL (ARM based) using one physical 32-bit timer. (WTIMER0A)
- AVR Atmega16/32 using 8-bit timer.(Timer0) 

## ToDo

- [ ] Port & Test it on Windows.