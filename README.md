Mervin
======

High Perfomance Trading Simulator and Execution engine
------------------------------------------------------

This is an extreme high performance set of library that allows to buils 
high performance trading simulation on data and then use same model to trade
live. 

It does not currently have a UI as it is intended to allow for extreme performance.

The basic architecture is a set of "core" functionality. 
The user must supply the actual model that acts on the data.
The system interfaces with LUA as a scripting language. This allows 
the developer to choose to move some of the functionality into a script file
which is convenient for optimization and optimization over cloud computing.

The system has been designed to be reentrant - thus allowing walk forward optimization 
while ruuning live. 

The system is built as a set of interfaces allowing the user to connect various pieces 
as need be. An example is the choice persistent position manager (better for production)
versus an in-memory position manager (better for optimization and simulation).

A few high performance optimizations have been performed in order to increase the speed of 
calculations. 

Currenlty the code can crunch about a million ticks a minute while performing HMMs and other 
advanced calculations. 

The system performs well interfacing to various data feeds and FIX providers. 

The commit process will be a little slow as to make sure that no proprietary information leaks 
out. In addition - more demos of connection types will be available. 

<more to follow>

What you will need to build:

For base example:
1. Lua 5.1 which you will need a library build in order to link with the system
2. QuickFix - if you want to connect to a third party using FIX
3. Some test data
4. I must have forgotton something


Why Open Source?
----------------

It's easier to maintain code that is open source, as it relieves me from the need to 
maintain the core code, and focus on the money making aspects of the system. 

License
-------

Mervin is made available under both the GPL v2 license and a BSD (3-point) style license. You can select which one you wish to use the DataTables code under.

Copyright (c) 2008-2012, Daniel Flam 
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of Daniel Flam nor NewYorkBrass.com may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
