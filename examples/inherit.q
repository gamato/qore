#!/usr/bin/env qore
# 
# this file gives examples of how class inheritance in qore works
#
# QORE INHERITANCE
# when a class inherits another class, all of the methods of the base
# class are available in the subclass (inherting class).  If the 
# subclass defines methods with the same names as methods in base
# classes, then the subclasses methods will override the base class
# methods.
#
# MULTIPLE INHERITANCE
# a class can inherit from more than one class (called multiple 
# inheritance).  In this case, if there are base class methods with the
# same names, the class listed first in the inheritance list will take
# precedence over classes listed afterwards (breadth-first, left-to-
# right resolution order)
#
# "SPECIAL" METHODS, ::constructor(), ::destructor(), and ::copy() base 
# class methods will be automatically called when the equivalent 
# subclass methods are executed.
# In this case, a depth-first, left-to-right order is used for 
# constructor and copy methods, which means that the lowest-level
# base class constructors are called first, using a left-to-right
# order (the declaration order) for classes at the same level.
# destructors will be called in the reverse order.
#
# NOTE: base class special methods will only be called once, even if 
# the class has been inherited multiple times in the hierarchy (for
# example, if a subclass "Final" inherits classes "Base1" and "Mid", 
# but "Base1" is already a subclass of class "Mid", the constructor 
# for "Base1" will only be executed once when the Final::constructor()
# method is executed
#
# NOTE: base class constructor arguments can be overridden by subclasses.
# if a subclass specifies arguments for base class, and another inherited
# class also specifies arguments for the base class, the lowest level 
# class' base constructor argument specification will be used (and the
# other class' base class arguments will not be evaluated in this case)
#
# NOTE: the other "special" methods ::methodGate() and ::memberGate() 
# are not called for base classes, subclasses have to define their own
# versions of these methods.  However, these special methods can be 
# explicitly called from subclasses

# require global variables to be declared with "our" before use
%require-our

# Base1 will be a base class
class Base1 {
    # declare some private members
    private $.base1, $.x;

    constructor($a)
    {
	printf("Base1::constructor(%n)\n", $a);
	$.a = $a;
    }
    destructor()
    {
	printf("Base1::destructor() (%n)\n", $.a);
    }
    copy()
    {
	printf("Base1::copy() (%n)\n", $.a);
	$.a = $.a + "-copy";
    }
    hello()
    {
	printf("Base1 hello (%n)\n", $.a);
    }
    Base1($a)
    {
	printf("Base1::Base1() (%n) %n\n", $.a, $a);
    }
}

# Base2 is a base class
class Base2 {
    # declare some private members
    private $.base2, $.y;

    constructor($a)
    {
	printf("Base2::constructor(%n)\n", $a);
	$.b = $a;
    }
    copy()
    {
	printf("Base2::copy() (%n)\n", $.b);
	$.b = $.b + "-copy"; 
    }
    destructor()
    {
	printf("Base2::destructor() (%n)\n", $.b);
    }
    hello()
    {
	printf("Base2 hello (%n)\n", $.b);
    }
    Base2($a)
    {
	printf("Base2::Base2() (%n) %n\n", $.b, $a);
    }
}

# Mid is a subclass of Base1 and Base2 (and will be a base class for 
# class Final)
namespace Mid;
class Mid::Mid inherits Base1, Base2
{
    # declare some private members
    private $.mid, $.z;

    constructor($m) : Base1("Mid->Base1-" + $m), Base2("Mid->Base2-" + $m)
    {
	printf("Mid::constructor(%n)\n", $m);
	$.m = $m;
    }
    copy()
    {
	printf("Mid::copy() (%n)\n", $.m);
	$.m = $.m + "-copy";
    }
    destructor()
    {
	printf("Mid::destructor() (%n)\n", $.m);
    }
    hello()
    {
	Base1::$.hello();
	Base2::$.hello();
	printf("Mid hello (%n)\n", $.m);
    }
    Mid($a)
    {
	$.Base1("Mid");
	$.Base2("Mid");
	printf("Mid::Mid() (%n) %n\n", $.m, $a);
    }
}

# class Final is a subclass of Mid, Base1, and Base2 (even though
# Base1 and Base2 are already subclasses of Mid)
# private members of this class: $.base1, $.base2, $.x, $.y, $.z
class Final inherits Mid::Mid, Base1, Base2;

Final::constructor($a) : Mid("Final->Mid-" + $a), Base1("Final->Base1-" + $a)
{
    printf("Final::constructor(%n)\n", $a);
    $.f = $a;
}

Final::copy()
{
    printf("Final::copy() (%n)\n", $.f);
    $.f = $.f + "-copy";
}

Final::destructor()
{
    printf("Final::destructor() (%n)\n", $.f);
}

Final::hello()
{
    # here we make a copy of ourselves and print out the result
    my $x = $.copy();
    printf("x=%N\n", $x);

    Mid::$.hello();
    Base1::$.hello();
    Base2::$.hello();
    $.Base1("Final");
    $.Base2("Final");
    $.Mid("Final");
    printf("Final hello\n");
}

# test constructors
#$m = new Mid("Mid");
#thread_exit;
our $a = new Final("Final");
# execute a subclass method
$a.hello();
# test that base class methods are available in the subclass
$a.Base1("top");
$a.Base2("top");
$a.Mid("top");

# objects are always references (unless explicitly copied), so $b and $a will represent the same object after the following assignment
our $b = $a;

# however, we can make an explicit copy
our $c = $a.copy();

our $x = new Final("hi");
# execute destructors as objects go out of scope
