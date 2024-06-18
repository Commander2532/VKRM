	.cdecls C,NOLIST
	%{
		#include <main.h>
		#include <config.h>
	%}

	.ref ||raw_rx_buffer||
	.ref ||rx_pointer||
	.ref ||rx_timeout||
	.ref ||rx_delay||
	.ref ||rx_count||
	.ref ||__delay_loop||

;*----------------------------------------------------------------------------------------------------*
;* Специализированные регистры:
;* R2 - регистр с адресом стека
;* R3 - регистр для хранения адресов программы
;* R14 - регистр для передачи аргументов между функциями
;* R30 - регистр для управления выводами процессора
;* R31 - регистр состояния (в т.ч. вводов процессора)
;*----------------------------------------------------------------------------------------------------*

	.sect ".text:__receive_byte"
	.clink
	.global ||__receive_byte||

;*----------------------------------------------------------------------------------------------------*
;* unsigned char __receive_byte(void)
;*
;* @ Brief:  Функция принимает 1 байт по протоколу SBNI с заранее заданной скоростью
;* @ Param:  Нет
;* @ Return: Принятый байт
;*----------------------------------------------------------------------------------------------------*

||__receive_byte||:
;*----------------------------------------------------------------------------------------------------*
	AND		r5, r31, SBNI_RX			; зафиксировать уровень на входе
	SUB		r2, r2, 2					; выделить память в стеке
	SBBO	&r3.w2, r2, 0, 2			; сохранить адрес возврата
	ZERO	&r7, 12						; инициализировать счетчик
	LDI		r0, ||rx_pointer||			; загрузить адрес rx_pointer
	LBBO	&r10, r0, 0, 4				; загрузить значение rx_pointer
	LDI		r0, ||rx_count||			; загрузить адрес recv_count
	LBBO	&r11, r0, 0, 4				; загрузить значение recv_count
	LDI		r0, ||rx_delay||			; загрузить адрес delay_rx
	LBBO	&r14, r0, 0, 4				; загрузить значение delay_rx
	SUB		r14, r14, 6					; скорректировать delay_rx
$RXL_0:
	AND		r5, r31, SBNI_RX			; зафиксировать уровень
	QBEQ	$RXL_6, r5, r6				; перейти к $RXL_6 если изменения уровня не произошло
	LDI		r0, ||rx_delay||  			; загрузить адрес delay_rx
	LBBO	&r14, r0, 0, 4				; загрузить значение delay_rx
$RXL_1:
	JAL		r3.w2, ||__delay_loop||		; вызвать __delay_loop
	AND		r6, r31, SBNI_RX			; зафиксировать уровень на входе
	XOR		r5, r5, r6					; XOR между значениями уровней на входе
	LSL		r8, r8, 1					; сдвинуть результат на 1 бит влево
	QBEQ	$RXL_2, r5, 0				; если изменения уровня не было перейти к $RXL_2
	ADD		r8.b0, r8.b0, 1				; иначе прибавить 1 к результату
$RXL_2:
	ADD		r7, r7, 1					; инкрементировать счётчик принятых бит
	QBEQ	$RXL_4, r7, 8				; перейти к $RXL_4, если счётчик равен 8
	LDI		r0, ||rx_timeout||			; загрузить адрес rx_timeout
	LBBO	&r1, r0, 0, 4				; загрузить значение rx_timeout в счётчик
$RXL_3:
	AND		r5, r31, SBNI_RX			; зафиксировать уровень на входе
	QBNE	$RXL_0, r6, r5				; перейти к $RXL_0 если произошло изменение уровня
	SUB		r1, r1, 1					; иначе уменьшить счётчик
	QBNE	$RXL_3, r1, 0				; если счётчик > 0 вернуться к $RXL_3
	JMP		$RXL_6						; иначе перейти к $RXL_6
$RXL_4:
	LDI		r0, BUFFER_SIZE				; поместить в r0 значение BUFFER_SIZE
	SUB		r0, r0, r10					; сравнить текущее значение rx_pointer с BUFFER_SIZE
	QBNE	$RXL_5, r0, 0				; если rx_pointer != BUFFER_SIZE перейти к $RXL_5
	LDI32	r10, 0						; иначе закольцевать буфер
$RXL_5:
	LDI32	r0, ||raw_rx_buffer||		; загрузить указатель на буфер
	SBBO	&r8.b0, r0, r10, 1			; сохранить байт по адресу rx_pointer с текущим сдвигом
	ADD		r10, r10, 1					; переместить rx_pointer на следующий байт
	ADD		r11, r11, 1					; увеличить счётчик принятых байт
	ZERO	&r7, 4						; обнулить счётчик принятых бит
	JMP		$RXL_3						; перейти к $RXL_3
$RXL_6:
	LBBO	&r3.w2, r2, 0, 2			; загрузить адрес возврата
	ADD		r2, r2, 2					; освободить память в стеке
	JMP		r3.w2						; перейти к адресу возврата
;*----------------------------------------------------------------------------------------------------*