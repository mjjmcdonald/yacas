---
layout: page
title: Big changes ahead for Yacas
author: ayalpinkus
---

Some of you may have noticed changes that have been made to the Yacas
project recently. The structure and look of the web site has changed a
lot, as have the contents of the downloadable version of Yacas. The
sharp observer might even have noticed that the release discipline has
changed too. Yacas had collected a lot of unnecessary weight over the
years, and there was room for improvements to the ease of use of
Yacas.

The original idea behind open-sourcing Yacas was always to get people
to help extend it. My reasoning was that if I wanted such a system,
other people were bound to also want it. Wouldn't it be great if all
those people helped me extend Yacas, instead of them starting their
own system? All I had to do is stop them from starting their own
project and work on mine instead. They would thus extend Yacas,
improve on it, making the system more valuable as a whole to every one
else including me. So Yacas started life as an open-source project,
and people did come to help.

In retrospect it is a wonder that people came to help at all! Consider
the effort one has to take in order to be able to contribute to
Yacas. First, one has to be able to use it, which means:

* have an interest in math (1 percent of the population, say)
* be comfortable using a Unix-style system (10 percent of the
  remaining population, say)
* download Yacas and compile it yourself, and install it
* learn a completely new programming language
* after *all* of that, you are greeted with a blinking cursor,
  and you are supposed to think up calculation examples yourself. You
  have to do all this work, and after that the benefits are not clear,
  you have to think of what you want to calculate.

Now, that was just to *use* Yacas. Now, if you want to actually
*contribute*, you have to:

* get comfortable with using the complex command-line tools called cvs and ssh
* get aqcuainted with the Yacas system
* get an account at Sourceforge (will take a day to process)
* get me to give you write permissions to the cvs repository (again
  another day of waiting)

People had to be really persistent, they had to really really want to
help extend Yacas. It is a miracle people came to help at all.

## User interface design to the rescue

User interface design is generally frowned upon in academia. It is not
'smart', does not require 'brains'. If you look at what equivalent
projects work on they very rarely spend much time considering the
interface to the user, the way the end user is going to communicate
with the system. There is a command line, and you type in commands
there. If you don't know a command, you look it up in a manual, or
worse, the source code because the manual is non-existent or
incomplete or not kept up to date. I have seen Google summer of code
projects for competing software programs where they sollicited help
from computer science students to design a user interface for
them. This is akin to, say, asking a student of mathematics to become
a cook in a one-star restaurant, <b>tonight</b>, or a photographer, or
an artist painter, etcetera. No amount of smarts will allow you to do
work instantly that requires skill and years of training. And user
interface design is an art, a skill, and it requires knowledge and
experience. You can not substitute smartness for that.
  

## But we are not alone any more!

There are two big competing computer algebra systems that Yacas
competes with, Axiom and Maxima. Axiom and Maxima have their roots in
the seventies, in an era where artificial intelligence was the next
big thing. Computers had to second-guess you. You typed in a problem,
and the system presented you with the solution. People have come to
think of computer algebra systems in that way, as huge databases of
mathematical knowledge to query, a search engine of mathematics. That
is what they mean when they say Yacas is less powerful than the other
systems. People type in some calculation that Axiom and Maxima had
entries for in their databases, and Yacas doesn't. These two systems
have math knowledge databases that are more extensive than the
database of knowledge in Yacas.

When I started Yacas back in 1998, there were no big open-source
computer algebra systems. People tend to forget that now with Maxima
and Axiom available in open-source form. But we used to be alone. We
were this little rowing boat on a big ocean. Suddenly two huge
titanic-sized ships named Axiom and Maxima came and floated next to
us. Two huge ships faring next to us, and we are in the middle with
our tiny little rowing boat named Yacas. These big ships suddenly
showed up out of the blue!

The event of the arrival of two big competitors forced me to consider
how to compete with them. Do we compete on their turf, or do we do
something else? The two classic ways to compete are to compete on
features (who has more features?), or to compete by providing
something different. Maxima and Axiom have approximately 30 years
under their belts. Hundreds of researchers have worked on it. They
have both been commercial. Hundreds of man-years of work went in to
them. Competing with them on their own turf might be a bad
idea. Maxima is understood to be the most powerful system, but Axiom
has another dream. Axioms dream is based on strong typing, and the
promise of mathematically provable correct code as a
consequence. Equally, Yacas will also have to find its niche, its area
where it outshines the other systems, if it is to have a reason to
exist.

Some other systems are focused on mathematical calculations of the
brute-force kind. You have some simple boring repetitive task, and
write a computer program specifically optimized for that calculation,
and let the computer do the dull grunt work, because that is what
computers are good at. This is not something Yacas will typically
shine at, since clean code and maintainability are considered more
important than raw performance in the Yacas project.

The thing Yacas development will focus on is ease of use. Ease of use
is not something you can casually fake. If this is done right other
systems can not easily copy it, because making a software program easy
to use is hard work. You have to think about the computers people use,
the software they have running on it, the operating system they run it
on, the tasks they would typically want to perform, their level of
knowledge of math and computer-related subjects, even the fonts they
have installed. You want to make it as easy as possible for them to
start using the system, preferably without having to install
software. The web site and application have to be responsive. When
they have the system running they should be able to do what they want
to do without having to learn too much about the system. People will
generally want to do a calculation, or learn some new
mathematics. They don't want to learn Yacas-specific details per
se. Using Yacas should be effortless in that they have to invest very
little in order to start using it.

In a sense, ease of use has always been a focus for Yacas. I always
wanted the system to be self-contained, easy to download, easy to
install without external dependencies. Yacas was as self-contained as
possible. It used its own numerics library, its own array and linked
list classes, all so it would not have to depend on external
libraries. After installation, the ideal was to only have to write a
few lines of code to do complex calculations, in an easy to read
programming language. Those are ease-of-use issues, and I have cared
for them from the very inception of Yacas.

## Javascript to the rescue

For the recent update of the web site I decided that we should use
Javascript to make Yacas easier to use. Javascript is now supported by
most browsers, it has been around for a long while, so almost every
one has access to a computer with a browser with Javascript
support. Javascript allows you to make dynamic web pages, where an
application runs inside a web browser without needing a constant link
to a web server to serve it pages. This causes the web page to feel
much more responsive than if it had to get every page through a
request to a server. The "on-line application" can be used
immediately, without having to download or install anything, thereby
removing one hurdle for people to start using the it. They can just
run Yacas on-line, from any OS that has a Java and Javascript-capable
browser. When entering the Yacas site, one is now one click away from
actually using Yacas.

Javascript is used in combination with the Yacas Java applet to bring
the on-line version of Yacas to life. One can now program on-line and
run your program directly in the Java applet inside your browser,
emulating the work-flow of an experienced user of Yacas. In addition,
the Yacas web site now also offers an interactive tutorial using
Javascript and Java to bring the examples to life. Also, there is a
find utility written in Javascript which allows the user to find
functions with accompanying descriptions and examples. The find
utility allows the user to use the system without having to remember
too much. Functions and their calling sequences can be looked up very
quickly.


## The Big Cleanup

For the long term one has to worry about the maintainability of a
system, if the system is to survive over the long haul. Will it still
be useful in a few years time? Will it still run on future systems?
Will it be easy to maintain by future generations?

The development mailing lists of the two systems Maxima and Axiom are
interesting reads. Essentially, you have the next generation of
maintainers of these systems communicating with each other there. The
original authors have generally moved to greener pastures (or
retired?). You have to understand that these systems are
<strong>huge</strong>, and mathematical algorithms are (or can be)
highly non-trivial. Taking over maintenance of such a system is not
trivial! These more mature systems now seem to have run in to the
problem of succession, where the next generation has to take over and
maintain the code that was sometimes developed three decades ago, when
memory was at a premium and every byte counted and you would rather
use a variable name with one letter, and rather not have comments in
the code. Looking at these systems gives us a glance at what lies
ahead in the future of Yacas (if it survives the next 30 years like
these systems have, impressively).

I had the problem of maintainability and succession in small with the
Yacas project. I was always willing to give cvs write access to Yacas,
to people whose coding expertise I had not checked yet. This resulted
in people adding non-trivial parts to Yacas, which was really great
because it made the system do more. However, there were now parts of
the code I didn't know intimately. Sometimes I didn't agree with the
way it was implemented either. But now it was in the system. Leaving
it in was easier than removing it. This reduced maintainability from
my standpoint. It is easier to maintain code you wrote yourself than
it is to write code written by some one else, in another programming
language, in another coding style. Most of the people making
contributions have now left, leaving me with the task of maintaining
what they contributed. Maintenance of parts that I don't understand
will cause me to slow down as I have to delve in to understanding that
part.

Because the code became harder for me to maintain, I have decided to
do a big cleanup. If I am going to commit to maintaining this system
for the next 50 or so years, I should be allowed to decide on
*what* I will be maintaining. I need to be quick, flexible,
efficient. Yacas needs to grow, become more useful. That is not going
to happen if I get slowed down in to trying to understand obscure
code. I am in the process of removing things that we don't need. It is
funny to see that I can remove enormous amounts of files and Yacas
still does what it used to do! It still passes all the tests!
Apparently we didn't need plugins. Apparently we didn't need an
fltk-based user interface. The list goes on. Removing big parts will
also make it easier for others to understand the system, and will
hopefully make the succession problem less severe for Yacas in 50
years.

The policy for accepting contributions has changed also. I have become
more strict. I now reserve the right to referree, review and edit
everything that goes in to the Yacas branch that I maintain. Every one
is free to maintain a different branch, or an addition to Yacas, but
control over the branch that I maintain is going to rest with me.

Contributions are very welcome. If you have any suggestions for
improvements, please don't hesitate to send an email! This is about
making it easier for you (yes, you) after all. I can not promise that
I can accommodate every wish, but feedback on how the system can be
improved is much appreciated. Feedback is a contribution too. I am
interested in the types of calculations you want to do, problems you
experience while using Yacas either on-line or off-line, or even plain
simple bugs or typos in the manual. Yacas will change some more in the
near future, and it can improve a lot with your help. If you are a
good writer or expert on a specific topic in mathematics, contributed
articles are also very welcome.

Releases are now very frequent, with a new release being made a couple
of times each week. The idea behind this is that since there are a lot
of improvements in the system, user interface, web log, and
documentation and such, it would be a pity if others would have to
wait in order to be able to use the new and improved version. People
don't have to download and install anything any more in order to be
able to use Yacas on-line. People who visit the web site use the
version that is on the web site at that moment. Updating that version
frequently means that people other than me have instant access to the
latest version also.

## A final word

Big changes have been made recently to Yacas to improve its ease of
use and maintainability. I hope you enjoy the changes I made. The plan
is to step up the effort and make more improvements in the near
future. So watch this space!