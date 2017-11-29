  # labwork.S
  # Written 2015-2017 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.

  #
  # Please keep the two macros PUSH and POP unchanged
  #

.global delay
.global time2string
.global hexasc
.global enable_interrupt
.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm
  #
  # Please add your own code below this line
  #
  	.data
	.align 2
mytime:	.word 0x5957
timstr:	.ascii "text more text lots of text\0"
.text
main: 
# you can write your code for subroutine "hexasc" below this line
hexasc: 
	andi $t0, $a0, 0xF
	bge  $t0, 10, character
	nop
	add $v0, $t0, 48
	jr $ra
	nop 
	
character:
	add $v0, $t0, 55
	jr $ra
	nop
	
delay:
	add $t0, $zero, $zero 
    	j while
   	nop
while: 
	slti $t1, $a0, 1 # om ms > 0 -> forloop -> (0)
	addi $a0, $a0, -1
	addi $t0, $zero, 0 
 	beq $t1, 0, forloop
    	nop
	jr $ra 
	nop
forloop:	
	add $t0, $t0, $zero
	slti $t1, $t0, 4711 # om i < constant -> forts ->  (1)
	addi $t0, $t0, 1
	bne $t1, 0, forloop
	nop
	j while 
	nop 
	
time2string:
	# Push the return address from when we jump and link (jal) on line 38. 
	andi $t0, $a1, 0xffff
	beq $t0, 0, sayHi
	PUSH $ra
	
	addi $a0, $a0, 0
	
	PUSH $a0 	# 
	andi $a0, $a1, 0xF # Retrieve the four lsb of time-info 
	jal hexasc
	nop
	POP $a0
	sb $v0, 4($a0)
	
	PUSH $a0
	andi $a0, $a1, 0xF0
	srl $a0, $a0, 4 
	jal hexasc
	nop
	POP $a0 
	sb $v0, 3($a0)
	
	addi $t1, $0, 0x3A
	sb $t1, 2($a0)
	
	PUSH $a0 
	andi $a0, $a1, 0xF00
	srl $a0, $a0, 8 
	jal hexasc 
	nop
	POP $a0
	sb $v0, 1($a0)
	
	PUSH $a0
	andi $a0, $a1, 0xF000
	srl $a0, $a0, 12
	jal hexasc 
	nop
	POP $a0
	sb $v0, 0($a0)
	
	li $t0, 0x0
	sb $t0, 5($a0)
	
	POP $ra
	jr $ra
	nop
sayHi:
	addi $t0, $zero, 0x48
	sb $t0, 0($a0)
	
	addi $t0, $zero, 0x4F
	sb $t0, 1($a0)
	
	addi $t0, $zero, 0x55
	sb $t0, 2($a0)
	
	addi $t0, $zero, 0x52
	sb $t0, 3($a0)
    
    sb $zero, 4($a0)    
	
	
	jr $ra
	nop
enable_interrupt:
  ei
  jr $ra
