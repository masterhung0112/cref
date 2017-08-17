<?php
#define variable
$myvar;
$myvar = 10;
$myvar = 1234;
$myvar = 0b10;
$myvar = 0123;
$myvar = 0x1A;
$myfloat = 1.234;
$myfloat = 3e2;
$mybool = true;
$myNull = null;
$myInt = $myNull + 0;
$myBool = $myNull == true

$x = 4 ** 2; // 16
$x++;
$x--;
++$x;
--$x;
$x = ( 2 == 3 );
$x = ( 2 != 3 );
$x = ( 2 <> 3 );
$x = ( 2 === 3 );
$x = ( 2 !== 3 );
$x = ( 2 > 3 );
$x = ( 2  < 3 );
$x = ( 1 <=> 1 ); // 0
$x = ( 1 <=> 2 ); // -1
$x = ( 3 <=> 2 ); // 1

$heredoc = <<<LABEL
Heredoc (with parsing)
LABEL;

$nowdoc = <<<'LABEL'
nowdoc (without parsing)
LABEL;

// Unicode escape character, which provides the ability to embed
// UTF-8 encoded characters into strings
echo "\u{00C2A9}"; // c (copyright sign)

$c = 'Hello';
$c[0] = 'J'; // Jello

$a = 'test';
$b = 'test';
$c = ($a === $b); // true

$a = array(1,2,3);
$a = [1,2,3];
$a[] = 4; // $a[3]

$b = array('one' => 'a',
	'two' => 'b',
	'three' => 'c');
$c = array(0 => 0, 1 => 1, 2 => 2);
$e = array(5 => 5, 6); // 6 => 6
$d = array(0 => 1, 'foo' => 'bar');
$a = array( array('00', '01'), array('10', '11') );
$a[0][0] = '00';
$b = array('one' => array('00', '01'));
$b['one'][0];

if (true) {
} elseif (false) {
} else {
}

if (true):
elseif (false):
else:
endif;

switch($x) {
	case 1: break;
	case 2: break;
	default; break;
}

switch($x):
case 1: break;
case 2: break;
default:
endswitch;

$y = ($x == 1) ? 1 : 2;
($x == 1) ? $y = 1 : $y = 2;

$a = array(1,2,3);
foreach($a as $v) {
}

foreach ($a as $v) :
endforeach;

$a = array('one' => 1, 'two' => 2, 'three' => 3);
foreach($a as $k => $v) {
}

foreach($a as $k => $v):
endforeach;

function myArgs($y = 'Earth') {
	$num = func_num_args();
	$y = func_get_args(0);
}

function myArgs3(...$args) {
	foreach($args as $v) {

	}
}

$a = [1, 2 ,3];
myArgs3(...$a); // "123"

$say = function ($name) {

};
$say("Hello World");

$x = 1;
$y = 2;
$myClosure = function($z) use ($x, $y)
{
	return $x + $y + $z;
}
$myClosure(3);

function getNum()
{
	for ($i = 0; $i < 5; $i++) {
		yield $i;
	}
}

foreach (getNum() as $v) {

}

function countToFive() {
	yield 1;
	yield from [2, 3, 4];
	yield 5;
}

class MyRectangle {
	public $x = 5, $y = 10;
	const PI = 3.14;
	static $pi = 3.14;
	static function newArea($a) {
		// static::$pi
		return self::$pi * $a * $a;
	}

	function __construct() {
		$this->x = 20;
	}
	function __destruct() {}
	function newArea($a, $b) { return $a * $b; }
	function getArea() { return $this->newArea($this->x, $this->y); }
}
$r = new MyRectangle();

$obj = new class('Hi')
{
	public $x;
	pubic function __construct($a) {
		$this->x = $a;
	}
};
echo $obj->x; // "Hi"

class C { private $x = 'Hi'; }
$getC = function() { return $this->x; };
$getX = $getC->bindTo(new C, 'C');
echo $getX(); // Hi

// PHP 7
$getX = function() { return $this->x; };
$getX->call(new C);

final class NotExtendable
{
	final function notOverridable() {}
}

class Square {}
$s = new Square(5);
$s instanceof Square;

define('DEBUG', 1, true);
echo debug; // "1"
if (!defined('DEBUG')) {}

abstract class Shape {
	abstract class public myAbstract();
}

trait PrintFunctionality
{
	public function myPrint() { echo 'Hello'; }
}

class MyClass
{
	use PrintFunctionality;
}
$o = new MyClass
$o->myPrint();


function __autoload($classname)
{
	include $classname . '.php';
}

interface I {
	static function myArray(array $a): array;
}

class C implement I {
	static function myArray(array $a): array {
		return $a;
	}
}

declare(strict_types=1);
