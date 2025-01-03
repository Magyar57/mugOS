bits 32

section .text

; void halt();
global halt
halt:
	hlt
	ret
; END halt

; void terminate();
global terminate
terminate:
	.terminate:
	cli
	hlt
	jmp .terminate
	ret
; END terminate

; void disableInterrupts();
global disableInterrupts
disableInterrupts:
	cli
	ret
; END disableInterrupts

; void enableInterrupts();
global enableInterrupts
enableInterrupts:
	sti
	ret
; END enableInterrupts

; void callInterrupt(uint8_t vector);
global callInterrupt
callInterrupt:
	push ebp
	mov ebp, esp

	; Call interrupt using the jump table
	mov eax, [ebp+8]	; interrupt number
	mov ebx, [callInterrupt_JumpTable + 4*eax]
	jmp ebx

	after:

	leave
	ret
; END callInterrupt

; bool CPU_supportsCpuid();
global CPU_supportsCpuid
CPU_supportsCpuid:
	push ebp
	mov ebp, esp

	; move EFLAGS into eax
	pushfd
	pop eax

	; move (EFLAGS xor 'id flag') back to EFLAGS
	mov ecx, eax		; save eflags for later comparison
	xor eax, 1<<21		; toggle bit 21 (ID flag)
	push eax
	popfd				; eflags = eax

	; re-move EFLAGS into eax, and compare it with the previous value
	pushfd
	pop eax
	xor eax, ecx
	and eax, 1<<21		; if bit clear, both EFLAGS had the same id flag value (cpuid unsupported)
	shr eax, 21			; return value is on 8 bits

	leave
	ret
; END CPU_supportsCpuid

; void cpuidWrapper(int code, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);
; This function assumes that the cpuid instruction is supported. To test it, use CPUSupportsCpuid
global cpuidWrapper
cpuidWrapper:
	push ebp
	mov ebp, esp

	mov eax, [ebp+8]
	cpuid

	mov edi, [ebp+12] ; &eax
	mov [edi], eax
	mov edi, [ebp+16] ; &ebx
	mov [edi], ebx
	mov edi, [ebp+20] ; &ecx
	mov [edi], ecx
	mov edi, [ebp+24] ; &edx
	mov [edi], edx

	leave
	ret
; END cpuidWrapper

section .data

callInterrupt_JumpTable:
	dd callInterrupt_Interrupt0
	dd callInterrupt_Interrupt1
	dd callInterrupt_Interrupt2
	dd callInterrupt_Interrupt3
	dd callInterrupt_Interrupt4
	dd callInterrupt_Interrupt5
	dd callInterrupt_Interrupt6
	dd callInterrupt_Interrupt7
	dd callInterrupt_Interrupt8
	dd callInterrupt_Interrupt9
	dd callInterrupt_Interrupt10
	dd callInterrupt_Interrupt11
	dd callInterrupt_Interrupt12
	dd callInterrupt_Interrupt13
	dd callInterrupt_Interrupt14
	dd callInterrupt_Interrupt15
	dd callInterrupt_Interrupt16
	dd callInterrupt_Interrupt17
	dd callInterrupt_Interrupt18
	dd callInterrupt_Interrupt19
	dd callInterrupt_Interrupt20
	dd callInterrupt_Interrupt21
	dd callInterrupt_Interrupt22
	dd callInterrupt_Interrupt23
	dd callInterrupt_Interrupt24
	dd callInterrupt_Interrupt25
	dd callInterrupt_Interrupt26
	dd callInterrupt_Interrupt27
	dd callInterrupt_Interrupt28
	dd callInterrupt_Interrupt29
	dd callInterrupt_Interrupt30
	dd callInterrupt_Interrupt31
	dd callInterrupt_Interrupt32
	dd callInterrupt_Interrupt33
	dd callInterrupt_Interrupt34
	dd callInterrupt_Interrupt35
	dd callInterrupt_Interrupt36
	dd callInterrupt_Interrupt37
	dd callInterrupt_Interrupt38
	dd callInterrupt_Interrupt39
	dd callInterrupt_Interrupt40
	dd callInterrupt_Interrupt41
	dd callInterrupt_Interrupt42
	dd callInterrupt_Interrupt43
	dd callInterrupt_Interrupt44
	dd callInterrupt_Interrupt45
	dd callInterrupt_Interrupt46
	dd callInterrupt_Interrupt47
	dd callInterrupt_Interrupt48
	dd callInterrupt_Interrupt49
	dd callInterrupt_Interrupt50
	dd callInterrupt_Interrupt51
	dd callInterrupt_Interrupt52
	dd callInterrupt_Interrupt53
	dd callInterrupt_Interrupt54
	dd callInterrupt_Interrupt55
	dd callInterrupt_Interrupt56
	dd callInterrupt_Interrupt57
	dd callInterrupt_Interrupt58
	dd callInterrupt_Interrupt59
	dd callInterrupt_Interrupt60
	dd callInterrupt_Interrupt61
	dd callInterrupt_Interrupt62
	dd callInterrupt_Interrupt63
	dd callInterrupt_Interrupt64
	dd callInterrupt_Interrupt65
	dd callInterrupt_Interrupt66
	dd callInterrupt_Interrupt67
	dd callInterrupt_Interrupt68
	dd callInterrupt_Interrupt69
	dd callInterrupt_Interrupt70
	dd callInterrupt_Interrupt71
	dd callInterrupt_Interrupt72
	dd callInterrupt_Interrupt73
	dd callInterrupt_Interrupt74
	dd callInterrupt_Interrupt75
	dd callInterrupt_Interrupt76
	dd callInterrupt_Interrupt77
	dd callInterrupt_Interrupt78
	dd callInterrupt_Interrupt79
	dd callInterrupt_Interrupt80
	dd callInterrupt_Interrupt81
	dd callInterrupt_Interrupt82
	dd callInterrupt_Interrupt83
	dd callInterrupt_Interrupt84
	dd callInterrupt_Interrupt85
	dd callInterrupt_Interrupt86
	dd callInterrupt_Interrupt87
	dd callInterrupt_Interrupt88
	dd callInterrupt_Interrupt89
	dd callInterrupt_Interrupt90
	dd callInterrupt_Interrupt91
	dd callInterrupt_Interrupt92
	dd callInterrupt_Interrupt93
	dd callInterrupt_Interrupt94
	dd callInterrupt_Interrupt95
	dd callInterrupt_Interrupt96
	dd callInterrupt_Interrupt97
	dd callInterrupt_Interrupt98
	dd callInterrupt_Interrupt99
	dd callInterrupt_Interrupt100
	dd callInterrupt_Interrupt101
	dd callInterrupt_Interrupt102
	dd callInterrupt_Interrupt103
	dd callInterrupt_Interrupt104
	dd callInterrupt_Interrupt105
	dd callInterrupt_Interrupt106
	dd callInterrupt_Interrupt107
	dd callInterrupt_Interrupt108
	dd callInterrupt_Interrupt109
	dd callInterrupt_Interrupt110
	dd callInterrupt_Interrupt111
	dd callInterrupt_Interrupt112
	dd callInterrupt_Interrupt113
	dd callInterrupt_Interrupt114
	dd callInterrupt_Interrupt115
	dd callInterrupt_Interrupt116
	dd callInterrupt_Interrupt117
	dd callInterrupt_Interrupt118
	dd callInterrupt_Interrupt119
	dd callInterrupt_Interrupt120
	dd callInterrupt_Interrupt121
	dd callInterrupt_Interrupt122
	dd callInterrupt_Interrupt123
	dd callInterrupt_Interrupt124
	dd callInterrupt_Interrupt125
	dd callInterrupt_Interrupt126
	dd callInterrupt_Interrupt127
	dd callInterrupt_Interrupt128
	dd callInterrupt_Interrupt129
	dd callInterrupt_Interrupt130
	dd callInterrupt_Interrupt131
	dd callInterrupt_Interrupt132
	dd callInterrupt_Interrupt133
	dd callInterrupt_Interrupt134
	dd callInterrupt_Interrupt135
	dd callInterrupt_Interrupt136
	dd callInterrupt_Interrupt137
	dd callInterrupt_Interrupt138
	dd callInterrupt_Interrupt139
	dd callInterrupt_Interrupt140
	dd callInterrupt_Interrupt141
	dd callInterrupt_Interrupt142
	dd callInterrupt_Interrupt143
	dd callInterrupt_Interrupt144
	dd callInterrupt_Interrupt145
	dd callInterrupt_Interrupt146
	dd callInterrupt_Interrupt147
	dd callInterrupt_Interrupt148
	dd callInterrupt_Interrupt149
	dd callInterrupt_Interrupt150
	dd callInterrupt_Interrupt151
	dd callInterrupt_Interrupt152
	dd callInterrupt_Interrupt153
	dd callInterrupt_Interrupt154
	dd callInterrupt_Interrupt155
	dd callInterrupt_Interrupt156
	dd callInterrupt_Interrupt157
	dd callInterrupt_Interrupt158
	dd callInterrupt_Interrupt159
	dd callInterrupt_Interrupt160
	dd callInterrupt_Interrupt161
	dd callInterrupt_Interrupt162
	dd callInterrupt_Interrupt163
	dd callInterrupt_Interrupt164
	dd callInterrupt_Interrupt165
	dd callInterrupt_Interrupt166
	dd callInterrupt_Interrupt167
	dd callInterrupt_Interrupt168
	dd callInterrupt_Interrupt169
	dd callInterrupt_Interrupt170
	dd callInterrupt_Interrupt171
	dd callInterrupt_Interrupt172
	dd callInterrupt_Interrupt173
	dd callInterrupt_Interrupt174
	dd callInterrupt_Interrupt175
	dd callInterrupt_Interrupt176
	dd callInterrupt_Interrupt177
	dd callInterrupt_Interrupt178
	dd callInterrupt_Interrupt179
	dd callInterrupt_Interrupt180
	dd callInterrupt_Interrupt181
	dd callInterrupt_Interrupt182
	dd callInterrupt_Interrupt183
	dd callInterrupt_Interrupt184
	dd callInterrupt_Interrupt185
	dd callInterrupt_Interrupt186
	dd callInterrupt_Interrupt187
	dd callInterrupt_Interrupt188
	dd callInterrupt_Interrupt189
	dd callInterrupt_Interrupt190
	dd callInterrupt_Interrupt191
	dd callInterrupt_Interrupt192
	dd callInterrupt_Interrupt193
	dd callInterrupt_Interrupt194
	dd callInterrupt_Interrupt195
	dd callInterrupt_Interrupt196
	dd callInterrupt_Interrupt197
	dd callInterrupt_Interrupt198
	dd callInterrupt_Interrupt199
	dd callInterrupt_Interrupt200
	dd callInterrupt_Interrupt201
	dd callInterrupt_Interrupt202
	dd callInterrupt_Interrupt203
	dd callInterrupt_Interrupt204
	dd callInterrupt_Interrupt205
	dd callInterrupt_Interrupt206
	dd callInterrupt_Interrupt207
	dd callInterrupt_Interrupt208
	dd callInterrupt_Interrupt209
	dd callInterrupt_Interrupt210
	dd callInterrupt_Interrupt211
	dd callInterrupt_Interrupt212
	dd callInterrupt_Interrupt213
	dd callInterrupt_Interrupt214
	dd callInterrupt_Interrupt215
	dd callInterrupt_Interrupt216
	dd callInterrupt_Interrupt217
	dd callInterrupt_Interrupt218
	dd callInterrupt_Interrupt219
	dd callInterrupt_Interrupt220
	dd callInterrupt_Interrupt221
	dd callInterrupt_Interrupt222
	dd callInterrupt_Interrupt223
	dd callInterrupt_Interrupt224
	dd callInterrupt_Interrupt225
	dd callInterrupt_Interrupt226
	dd callInterrupt_Interrupt227
	dd callInterrupt_Interrupt228
	dd callInterrupt_Interrupt229
	dd callInterrupt_Interrupt230
	dd callInterrupt_Interrupt231
	dd callInterrupt_Interrupt232
	dd callInterrupt_Interrupt233
	dd callInterrupt_Interrupt234
	dd callInterrupt_Interrupt235
	dd callInterrupt_Interrupt236
	dd callInterrupt_Interrupt237
	dd callInterrupt_Interrupt238
	dd callInterrupt_Interrupt239
	dd callInterrupt_Interrupt240
	dd callInterrupt_Interrupt241
	dd callInterrupt_Interrupt242
	dd callInterrupt_Interrupt243
	dd callInterrupt_Interrupt244
	dd callInterrupt_Interrupt245
	dd callInterrupt_Interrupt246
	dd callInterrupt_Interrupt247
	dd callInterrupt_Interrupt248
	dd callInterrupt_Interrupt249
	dd callInterrupt_Interrupt250
	dd callInterrupt_Interrupt251
	dd callInterrupt_Interrupt252
	dd callInterrupt_Interrupt253
	dd callInterrupt_Interrupt254
	dd callInterrupt_Interrupt255
; END CallInterrupt_JumpTable

section .text

; CallInterrupt Interrupts calls
	callInterrupt_Interrupt0:
		int 0x0
		jmp after
	callInterrupt_Interrupt1:
		int 0x1
		jmp after
	callInterrupt_Interrupt2:
		int 0x2
		jmp after
	callInterrupt_Interrupt3:
		int 0x3
		jmp after
	callInterrupt_Interrupt4:
		int 0x4
		jmp after
	callInterrupt_Interrupt5:
		int 0x5
		jmp after
	callInterrupt_Interrupt6:
		int 0x6
		jmp after
	callInterrupt_Interrupt7:
		int 0x7
		jmp after
	callInterrupt_Interrupt8:
		int 0x8
		jmp after
	callInterrupt_Interrupt9:
		int 0x9
		jmp after
	callInterrupt_Interrupt10:
		int 0xa
		jmp after
	callInterrupt_Interrupt11:
		int 0xb
		jmp after
	callInterrupt_Interrupt12:
		int 0xc
		jmp after
	callInterrupt_Interrupt13:
		int 0xd
		jmp after
	callInterrupt_Interrupt14:
		int 0xe
		jmp after
	callInterrupt_Interrupt15:
		int 0xf
		jmp after
	callInterrupt_Interrupt16:
		int 0x10
		jmp after
	callInterrupt_Interrupt17:
		int 0x11
		jmp after
	callInterrupt_Interrupt18:
		int 0x12
		jmp after
	callInterrupt_Interrupt19:
		int 0x13
		jmp after
	callInterrupt_Interrupt20:
		int 0x14
		jmp after
	callInterrupt_Interrupt21:
		int 0x15
		jmp after
	callInterrupt_Interrupt22:
		int 0x16
		jmp after
	callInterrupt_Interrupt23:
		int 0x17
		jmp after
	callInterrupt_Interrupt24:
		int 0x18
		jmp after
	callInterrupt_Interrupt25:
		int 0x19
		jmp after
	callInterrupt_Interrupt26:
		int 0x1a
		jmp after
	callInterrupt_Interrupt27:
		int 0x1b
		jmp after
	callInterrupt_Interrupt28:
		int 0x1c
		jmp after
	callInterrupt_Interrupt29:
		int 0x1d
		jmp after
	callInterrupt_Interrupt30:
		int 0x1e
		jmp after
	callInterrupt_Interrupt31:
		int 0x1f
		jmp after
	callInterrupt_Interrupt32:
		int 0x20
		jmp after
	callInterrupt_Interrupt33:
		int 0x21
		jmp after
	callInterrupt_Interrupt34:
		int 0x22
		jmp after
	callInterrupt_Interrupt35:
		int 0x23
		jmp after
	callInterrupt_Interrupt36:
		int 0x24
		jmp after
	callInterrupt_Interrupt37:
		int 0x25
		jmp after
	callInterrupt_Interrupt38:
		int 0x26
		jmp after
	callInterrupt_Interrupt39:
		int 0x27
		jmp after
	callInterrupt_Interrupt40:
		int 0x28
		jmp after
	callInterrupt_Interrupt41:
		int 0x29
		jmp after
	callInterrupt_Interrupt42:
		int 0x2a
		jmp after
	callInterrupt_Interrupt43:
		int 0x2b
		jmp after
	callInterrupt_Interrupt44:
		int 0x2c
		jmp after
	callInterrupt_Interrupt45:
		int 0x2d
		jmp after
	callInterrupt_Interrupt46:
		int 0x2e
		jmp after
	callInterrupt_Interrupt47:
		int 0x2f
		jmp after
	callInterrupt_Interrupt48:
		int 0x30
		jmp after
	callInterrupt_Interrupt49:
		int 0x31
		jmp after
	callInterrupt_Interrupt50:
		int 0x32
		jmp after
	callInterrupt_Interrupt51:
		int 0x33
		jmp after
	callInterrupt_Interrupt52:
		int 0x34
		jmp after
	callInterrupt_Interrupt53:
		int 0x35
		jmp after
	callInterrupt_Interrupt54:
		int 0x36
		jmp after
	callInterrupt_Interrupt55:
		int 0x37
		jmp after
	callInterrupt_Interrupt56:
		int 0x38
		jmp after
	callInterrupt_Interrupt57:
		int 0x39
		jmp after
	callInterrupt_Interrupt58:
		int 0x3a
		jmp after
	callInterrupt_Interrupt59:
		int 0x3b
		jmp after
	callInterrupt_Interrupt60:
		int 0x3c
		jmp after
	callInterrupt_Interrupt61:
		int 0x3d
		jmp after
	callInterrupt_Interrupt62:
		int 0x3e
		jmp after
	callInterrupt_Interrupt63:
		int 0x3f
		jmp after
	callInterrupt_Interrupt64:
		int 0x40
		jmp after
	callInterrupt_Interrupt65:
		int 0x41
		jmp after
	callInterrupt_Interrupt66:
		int 0x42
		jmp after
	callInterrupt_Interrupt67:
		int 0x43
		jmp after
	callInterrupt_Interrupt68:
		int 0x44
		jmp after
	callInterrupt_Interrupt69:
		int 0x45
		jmp after
	callInterrupt_Interrupt70:
		int 0x46
		jmp after
	callInterrupt_Interrupt71:
		int 0x47
		jmp after
	callInterrupt_Interrupt72:
		int 0x48
		jmp after
	callInterrupt_Interrupt73:
		int 0x49
		jmp after
	callInterrupt_Interrupt74:
		int 0x4a
		jmp after
	callInterrupt_Interrupt75:
		int 0x4b
		jmp after
	callInterrupt_Interrupt76:
		int 0x4c
		jmp after
	callInterrupt_Interrupt77:
		int 0x4d
		jmp after
	callInterrupt_Interrupt78:
		int 0x4e
		jmp after
	callInterrupt_Interrupt79:
		int 0x4f
		jmp after
	callInterrupt_Interrupt80:
		int 0x50
		jmp after
	callInterrupt_Interrupt81:
		int 0x51
		jmp after
	callInterrupt_Interrupt82:
		int 0x52
		jmp after
	callInterrupt_Interrupt83:
		int 0x53
		jmp after
	callInterrupt_Interrupt84:
		int 0x54
		jmp after
	callInterrupt_Interrupt85:
		int 0x55
		jmp after
	callInterrupt_Interrupt86:
		int 0x56
		jmp after
	callInterrupt_Interrupt87:
		int 0x57
		jmp after
	callInterrupt_Interrupt88:
		int 0x58
		jmp after
	callInterrupt_Interrupt89:
		int 0x59
		jmp after
	callInterrupt_Interrupt90:
		int 0x5a
		jmp after
	callInterrupt_Interrupt91:
		int 0x5b
		jmp after
	callInterrupt_Interrupt92:
		int 0x5c
		jmp after
	callInterrupt_Interrupt93:
		int 0x5d
		jmp after
	callInterrupt_Interrupt94:
		int 0x5e
		jmp after
	callInterrupt_Interrupt95:
		int 0x5f
		jmp after
	callInterrupt_Interrupt96:
		int 0x60
		jmp after
	callInterrupt_Interrupt97:
		int 0x61
		jmp after
	callInterrupt_Interrupt98:
		int 0x62
		jmp after
	callInterrupt_Interrupt99:
		int 0x63
		jmp after
	callInterrupt_Interrupt100:
		int 0x64
		jmp after
	callInterrupt_Interrupt101:
		int 0x65
		jmp after
	callInterrupt_Interrupt102:
		int 0x66
		jmp after
	callInterrupt_Interrupt103:
		int 0x67
		jmp after
	callInterrupt_Interrupt104:
		int 0x68
		jmp after
	callInterrupt_Interrupt105:
		int 0x69
		jmp after
	callInterrupt_Interrupt106:
		int 0x6a
		jmp after
	callInterrupt_Interrupt107:
		int 0x6b
		jmp after
	callInterrupt_Interrupt108:
		int 0x6c
		jmp after
	callInterrupt_Interrupt109:
		int 0x6d
		jmp after
	callInterrupt_Interrupt110:
		int 0x6e
		jmp after
	callInterrupt_Interrupt111:
		int 0x6f
		jmp after
	callInterrupt_Interrupt112:
		int 0x70
		jmp after
	callInterrupt_Interrupt113:
		int 0x71
		jmp after
	callInterrupt_Interrupt114:
		int 0x72
		jmp after
	callInterrupt_Interrupt115:
		int 0x73
		jmp after
	callInterrupt_Interrupt116:
		int 0x74
		jmp after
	callInterrupt_Interrupt117:
		int 0x75
		jmp after
	callInterrupt_Interrupt118:
		int 0x76
		jmp after
	callInterrupt_Interrupt119:
		int 0x77
		jmp after
	callInterrupt_Interrupt120:
		int 0x78
		jmp after
	callInterrupt_Interrupt121:
		int 0x79
		jmp after
	callInterrupt_Interrupt122:
		int 0x7a
		jmp after
	callInterrupt_Interrupt123:
		int 0x7b
		jmp after
	callInterrupt_Interrupt124:
		int 0x7c
		jmp after
	callInterrupt_Interrupt125:
		int 0x7d
		jmp after
	callInterrupt_Interrupt126:
		int 0x7e
		jmp after
	callInterrupt_Interrupt127:
		int 0x7f
		jmp after
	callInterrupt_Interrupt128:
		int 0x80
		jmp after
	callInterrupt_Interrupt129:
		int 0x81
		jmp after
	callInterrupt_Interrupt130:
		int 0x82
		jmp after
	callInterrupt_Interrupt131:
		int 0x83
		jmp after
	callInterrupt_Interrupt132:
		int 0x84
		jmp after
	callInterrupt_Interrupt133:
		int 0x85
		jmp after
	callInterrupt_Interrupt134:
		int 0x86
		jmp after
	callInterrupt_Interrupt135:
		int 0x87
		jmp after
	callInterrupt_Interrupt136:
		int 0x88
		jmp after
	callInterrupt_Interrupt137:
		int 0x89
		jmp after
	callInterrupt_Interrupt138:
		int 0x8a
		jmp after
	callInterrupt_Interrupt139:
		int 0x8b
		jmp after
	callInterrupt_Interrupt140:
		int 0x8c
		jmp after
	callInterrupt_Interrupt141:
		int 0x8d
		jmp after
	callInterrupt_Interrupt142:
		int 0x8e
		jmp after
	callInterrupt_Interrupt143:
		int 0x8f
		jmp after
	callInterrupt_Interrupt144:
		int 0x90
		jmp after
	callInterrupt_Interrupt145:
		int 0x91
		jmp after
	callInterrupt_Interrupt146:
		int 0x92
		jmp after
	callInterrupt_Interrupt147:
		int 0x93
		jmp after
	callInterrupt_Interrupt148:
		int 0x94
		jmp after
	callInterrupt_Interrupt149:
		int 0x95
		jmp after
	callInterrupt_Interrupt150:
		int 0x96
		jmp after
	callInterrupt_Interrupt151:
		int 0x97
		jmp after
	callInterrupt_Interrupt152:
		int 0x98
		jmp after
	callInterrupt_Interrupt153:
		int 0x99
		jmp after
	callInterrupt_Interrupt154:
		int 0x9a
		jmp after
	callInterrupt_Interrupt155:
		int 0x9b
		jmp after
	callInterrupt_Interrupt156:
		int 0x9c
		jmp after
	callInterrupt_Interrupt157:
		int 0x9d
		jmp after
	callInterrupt_Interrupt158:
		int 0x9e
		jmp after
	callInterrupt_Interrupt159:
		int 0x9f
		jmp after
	callInterrupt_Interrupt160:
		int 0xa0
		jmp after
	callInterrupt_Interrupt161:
		int 0xa1
		jmp after
	callInterrupt_Interrupt162:
		int 0xa2
		jmp after
	callInterrupt_Interrupt163:
		int 0xa3
		jmp after
	callInterrupt_Interrupt164:
		int 0xa4
		jmp after
	callInterrupt_Interrupt165:
		int 0xa5
		jmp after
	callInterrupt_Interrupt166:
		int 0xa6
		jmp after
	callInterrupt_Interrupt167:
		int 0xa7
		jmp after
	callInterrupt_Interrupt168:
		int 0xa8
		jmp after
	callInterrupt_Interrupt169:
		int 0xa9
		jmp after
	callInterrupt_Interrupt170:
		int 0xaa
		jmp after
	callInterrupt_Interrupt171:
		int 0xab
		jmp after
	callInterrupt_Interrupt172:
		int 0xac
		jmp after
	callInterrupt_Interrupt173:
		int 0xad
		jmp after
	callInterrupt_Interrupt174:
		int 0xae
		jmp after
	callInterrupt_Interrupt175:
		int 0xaf
		jmp after
	callInterrupt_Interrupt176:
		int 0xb0
		jmp after
	callInterrupt_Interrupt177:
		int 0xb1
		jmp after
	callInterrupt_Interrupt178:
		int 0xb2
		jmp after
	callInterrupt_Interrupt179:
		int 0xb3
		jmp after
	callInterrupt_Interrupt180:
		int 0xb4
		jmp after
	callInterrupt_Interrupt181:
		int 0xb5
		jmp after
	callInterrupt_Interrupt182:
		int 0xb6
		jmp after
	callInterrupt_Interrupt183:
		int 0xb7
		jmp after
	callInterrupt_Interrupt184:
		int 0xb8
		jmp after
	callInterrupt_Interrupt185:
		int 0xb9
		jmp after
	callInterrupt_Interrupt186:
		int 0xba
		jmp after
	callInterrupt_Interrupt187:
		int 0xbb
		jmp after
	callInterrupt_Interrupt188:
		int 0xbc
		jmp after
	callInterrupt_Interrupt189:
		int 0xbd
		jmp after
	callInterrupt_Interrupt190:
		int 0xbe
		jmp after
	callInterrupt_Interrupt191:
		int 0xbf
		jmp after
	callInterrupt_Interrupt192:
		int 0xc0
		jmp after
	callInterrupt_Interrupt193:
		int 0xc1
		jmp after
	callInterrupt_Interrupt194:
		int 0xc2
		jmp after
	callInterrupt_Interrupt195:
		int 0xc3
		jmp after
	callInterrupt_Interrupt196:
		int 0xc4
		jmp after
	callInterrupt_Interrupt197:
		int 0xc5
		jmp after
	callInterrupt_Interrupt198:
		int 0xc6
		jmp after
	callInterrupt_Interrupt199:
		int 0xc7
		jmp after
	callInterrupt_Interrupt200:
		int 0xc8
		jmp after
	callInterrupt_Interrupt201:
		int 0xc9
		jmp after
	callInterrupt_Interrupt202:
		int 0xca
		jmp after
	callInterrupt_Interrupt203:
		int 0xcb
		jmp after
	callInterrupt_Interrupt204:
		int 0xcc
		jmp after
	callInterrupt_Interrupt205:
		int 0xcd
		jmp after
	callInterrupt_Interrupt206:
		int 0xce
		jmp after
	callInterrupt_Interrupt207:
		int 0xcf
		jmp after
	callInterrupt_Interrupt208:
		int 0xd0
		jmp after
	callInterrupt_Interrupt209:
		int 0xd1
		jmp after
	callInterrupt_Interrupt210:
		int 0xd2
		jmp after
	callInterrupt_Interrupt211:
		int 0xd3
		jmp after
	callInterrupt_Interrupt212:
		int 0xd4
		jmp after
	callInterrupt_Interrupt213:
		int 0xd5
		jmp after
	callInterrupt_Interrupt214:
		int 0xd6
		jmp after
	callInterrupt_Interrupt215:
		int 0xd7
		jmp after
	callInterrupt_Interrupt216:
		int 0xd8
		jmp after
	callInterrupt_Interrupt217:
		int 0xd9
		jmp after
	callInterrupt_Interrupt218:
		int 0xda
		jmp after
	callInterrupt_Interrupt219:
		int 0xdb
		jmp after
	callInterrupt_Interrupt220:
		int 0xdc
		jmp after
	callInterrupt_Interrupt221:
		int 0xdd
		jmp after
	callInterrupt_Interrupt222:
		int 0xde
		jmp after
	callInterrupt_Interrupt223:
		int 0xdf
		jmp after
	callInterrupt_Interrupt224:
		int 0xe0
		jmp after
	callInterrupt_Interrupt225:
		int 0xe1
		jmp after
	callInterrupt_Interrupt226:
		int 0xe2
		jmp after
	callInterrupt_Interrupt227:
		int 0xe3
		jmp after
	callInterrupt_Interrupt228:
		int 0xe4
		jmp after
	callInterrupt_Interrupt229:
		int 0xe5
		jmp after
	callInterrupt_Interrupt230:
		int 0xe6
		jmp after
	callInterrupt_Interrupt231:
		int 0xe7
		jmp after
	callInterrupt_Interrupt232:
		int 0xe8
		jmp after
	callInterrupt_Interrupt233:
		int 0xe9
		jmp after
	callInterrupt_Interrupt234:
		int 0xea
		jmp after
	callInterrupt_Interrupt235:
		int 0xeb
		jmp after
	callInterrupt_Interrupt236:
		int 0xec
		jmp after
	callInterrupt_Interrupt237:
		int 0xed
		jmp after
	callInterrupt_Interrupt238:
		int 0xee
		jmp after
	callInterrupt_Interrupt239:
		int 0xef
		jmp after
	callInterrupt_Interrupt240:
		int 0xf0
		jmp after
	callInterrupt_Interrupt241:
		int 0xf1
		jmp after
	callInterrupt_Interrupt242:
		int 0xf2
		jmp after
	callInterrupt_Interrupt243:
		int 0xf3
		jmp after
	callInterrupt_Interrupt244:
		int 0xf4
		jmp after
	callInterrupt_Interrupt245:
		int 0xf5
		jmp after
	callInterrupt_Interrupt246:
		int 0xf6
		jmp after
	callInterrupt_Interrupt247:
		int 0xf7
		jmp after
	callInterrupt_Interrupt248:
		int 0xf8
		jmp after
	callInterrupt_Interrupt249:
		int 0xf9
		jmp after
	callInterrupt_Interrupt250:
		int 0xfa
		jmp after
	callInterrupt_Interrupt251:
		int 0xfb
		jmp after
	callInterrupt_Interrupt252:
		int 0xfc
		jmp after
	callInterrupt_Interrupt253:
		int 0xfd
		jmp after
	callInterrupt_Interrupt254:
		int 0xfe
		jmp after
	callInterrupt_Interrupt255:
		int 0xff
		jmp after
; end interrupts
