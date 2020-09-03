.data
hint: .asciiz "Enter an integer:"
Creturn: .asciiz "\n"
array: .space 40

.globl main0
.text
main0:	jal main
	li $v0, 10
	syscall

read:
	addi $sp, $sp, -4
	sw $ra, 0($sp)

	li $v0, 4
	la $a0, hint
	syscall
	li $v0, 5
	syscall

	lw $ra, 0($sp)
	addi $sp, $sp, 4
	jr $ra

write:
	addi $sp, $sp, -4
	sw $ra, 0($sp)

	li $v0, 1
	syscall
	li $v0, 4
	la $a0, Creturn
	syscall
	move $v0, $zero

	lw $ra, 0($sp)
	addi $sp, $sp, 4
	jr $ra

fibo:
	addi $sp, $sp, -68
	sw $t0, 64($sp)
	sw $t1, 60($sp)
	sw $t2, 56($sp)
	sw $t3, 52($sp)
	sw $t4, 48($sp)
	sw $t5, 44($sp)
	sw $t6, 40($sp)
	sw $t7, 36($sp)
	sw $s0, 32($sp)
	sw $s1, 28($sp)
	sw $s2, 24($sp)
	sw $s3, 20($sp)
	sw $s4, 16($sp)
	sw $s5, 12($sp)
	sw $s6, 8($sp)
	sw $s7, 4($sp)
	sw $ra, 0($sp)

	move $t0, $a0
	li $t1, 1
	beq $t0, $t1, label3
	j label4
label4:
	li $t1, 2
	beq $t0, $t1, label3
	j label2
label3:
	li $t1, 1
	move $v0, $t1

	lw $ra, 0($sp)
	lw $s7, 4($sp)
	lw $s6, 8($sp)
	lw $s5, 12($sp)
	lw $s4, 16($sp)
	lw $s3, 20($sp)
	lw $s2, 24($sp)
	lw $s1, 28($sp)
	lw $s0, 32($sp)
	lw $t7, 36($sp)
	lw $t6, 40($sp)
	lw $t5, 44($sp)
	lw $t4, 48($sp)
	lw $t3, 52($sp)
	lw $t2, 56($sp)
	lw $t1, 60($sp)
	lw $t0, 64($sp)
	addi $sp, $sp, 68
	jr $ra
label2:
	li $t1, 1
	sub $t2, $t0, $t1
	move $a0, $t2
	jal fibo
	move $t1, $v0
	li $t2, 2
	sub $t3, $t0, $t2
	move $a0, $t3
	jal fibo
	move $t2, $v0
	add $t3, $t1, $t2
	move $v0, $t3

	lw $ra, 0($sp)
	lw $s7, 4($sp)
	lw $s6, 8($sp)
	lw $s5, 12($sp)
	lw $s4, 16($sp)
	lw $s3, 20($sp)
	lw $s2, 24($sp)
	lw $s1, 28($sp)
	lw $s0, 32($sp)
	lw $t7, 36($sp)
	lw $t6, 40($sp)
	lw $t5, 44($sp)
	lw $t4, 48($sp)
	lw $t3, 52($sp)
	lw $t2, 56($sp)
	lw $t1, 60($sp)
	lw $t0, 64($sp)
	addi $sp, $sp, 68
	jr $ra
label1:

main:
	addi $sp, $sp, -4
	sw $ra, 0($sp)

	li $t1, 0
	move $t2, $t1
	jal read
	move $t1, $v0
	move $t3, $t1
	li $t1, 1
	move $t2, $t1
label10:
	ble $t2, $t3, label9
	j label8
label9:
	move $a0, $t2
	jal fibo
	move $t1, $v0
	la $t8, array
	mul $t9, $t2, 4
	add $t8, $t8, $t9
	sw $t1, 0($t8)
	la $t8, array
	mul $t9, $t2, 4
	add $t8, $t8, $t9
	lw $t1, 0($t8)
	move $a0, $t1
	jal write
	addi $t2, $t2, 1
	j label10
label8:
	move $t1, $t3
	move $a0, $t1
	jal write
	li $t4, 1
	move $v0, $t4

	lw $ra, 0($sp)
	addi $sp, $sp, 4
	jr $ra
label5: