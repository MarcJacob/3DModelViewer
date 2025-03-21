This is a work-in-progress general-purpose 3D model viewer.

The goal of the project is to build a command-line-usable tool to easily inspect a 3D model under one of the common formats.
Doing so, it should be as easy as possible to port to any platform through a strong layer of abstraction between the logic and the platform resources.

This is built for me to learn modern C++ programming. Coming from the video game industry, our way of writing C / C++ Code is a little insular and specific,
picking and choosing C++ features we choose to allow or not on a project by project basis, not to mention the use of Game Engines that can impose constraints...

This makes my own experience quite removed from a lot of the software industry, which is what I'm trying to correct.

Let's make one thing clear: if I had to build almost any sort of product, especially a game, I would ***NOT*** be making myself use modern C++ constructs like smart pointers, standardized thread objects, new / delete operators (and, god help me, overloads of those operators) and the like.

> My initial assumption as a video game programmer is that modern C++ has been designed for large programming teams that pressure their own employees to deliver too quickly, and whom have been force-fed the idea that object orientation, "clean code" and the like are the end-all-be-all since their early higher education. It works well in the context of someone arriving on a large codebase that's been poorly designed, and needs to have some new feature bolted on like a piece of armor plate on a Sedan before it's sent on some insurgency frontline. This is why there's such an emphasis on taking the management of memory (along with any other critical resource) away from the programmer's hands, and writing layer upon layer of indirection and relying as much as possible on a standard library. Thinking of every component as an individual *object* that will work within some incomprehensible ecosystem rather than thinking more holistically.

> "So why are you doing this to yourself ?" one may ask. Well, for one, employability it still important, and if the market expects me to show I *understand* these constructs, so be it. It's only fair I put in the effort. Second of, I realize my relative inexperience in the field and that especially now is a time I must challenge my preconceptions. If I end up producing anything here, maybe it will be enough to make me realize how wrong I was, and I'll grow as a programmer. And if it only strengthens my existing beliefs, the result is very much the same.

> "You're not actually writing modern C++ but your wierd take on it so that's why XXX sucks for you". Maybe ! I'll be happy to hear about what I have been misusing. I definitely realize there's a high change it will happen. Or maybe I am ignorant of something that exists that is supposed to make my life easier. But even after a few years programming professionally, let's just say I've lost all faith that anybody is writing code that truly follows modern standards. So what I'm building here might be pretty realistic in this aspect ;)

# OVERALL PROGRAM STRUCTURE

Structure of the program: 

- Engine: Implementation files and headers for the "Engine" part of the code, which is definable as the platform-independent code managing the bulk of the logic behind rendering and reacting to input.
- [Platform Name]: Platform implementation folder with both implementation files and internal headers.

# CODE SPECIFICATIONS

Specifications to follow, in no particular order:

- The code should never create dependencies between any part of the Engine and a specific platform !
- The Engine code should not make use of any static memory - the program's initial memory usage should only be what is expectable for a regular program on the target platform, along with whatever static memory the platform specific code wants to use.
- The Engine CAN contain code that is *usable* by specific platforms but not others. That code however should still not depend on any specific platform.
- Platform-specific global symbols and files have an appropriate prefix, even if a Namespace is used to wrap it. This is because even inside platform-specific code, we want to differentiate between engine code, standard library code and the actual platform-specific code.
- Platform-specific files should use snake_case while Engine files should use PascalCase.
- While the exact way the Engine runs is up to the platform implementation, Engine code & especially interactions with the platform should be written so that it can run in a different thread.

# NOTES & DOCUMENTATION

While my efforts to document the program have as yet been nil, you can still find some help exploring and understanding it - and more importantly, some of my thought processes - by searching for comments that start with a # such as #NOTE, #TODO, #TOTHINK... Most of the time they'll have my first name added as well, not that I anticipate anyone working on this codebase beside me, but simply because it's a good habit I want to keep.

#TODO: To be done later at some point. I leave those sparringly, mostly as something to lookup and find to guide my own efforts. They could most of the time be replaced by appropriate project management like a backlog or tickets.

#NOTE: A remark I want to leave so my thought process can be better understood, whenever I feel like I'm doing things in a way that might appear strange / arbitrary / wrong to a reader.

#TOTHINK: Not necessarily a *task* per se but something to think about later. Usually used to indicate that something crossed my mind that might make me change the way I've already done something later, but was of too little relevance at the time of writing the code. Can easily either be wiped out or turned into a #TODO.

#TEST: I don't *usually* leave those laying around in commited code but if they're here then usually they're meant to divert execution away from an undevelopped feature.
