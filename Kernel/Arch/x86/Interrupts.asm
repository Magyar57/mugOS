bits 32

section .text

; void x86_DisableInterrupts();
global x86_DisableInterrupts
x86_DisableInterrupts:
	cli
	ret
; END x86_DisableInterrupts

; void x86_EnableInterrupts();
global x86_EnableInterrupts
x86_EnableInterrupts:
	sti
	ret
; END x86_EnableInterrupts

; void x86_CallInterrupt(uint8_t vector);
global x86_CallInterrupt
x86_CallInterrupt:
	enter 0, 0

	; Call interrupt using the jump table
	mov eax, [ebp+8]	; interrupt number
	mov ebx, [x86_CallInterrupt_JumpTable + 4*eax]
	jmp ebx

	after:

	leave
	ret
; END x86_CallInterrupt

section .data

x86_CallInterrupt_JumpTable:
	dd x86_CallInterrupt_Interrupt0
	dd x86_CallInterrupt_Interrupt1
	dd x86_CallInterrupt_Interrupt2
	dd x86_CallInterrupt_Interrupt3
	dd x86_CallInterrupt_Interrupt4
	dd x86_CallInterrupt_Interrupt5
	dd x86_CallInterrupt_Interrupt6
	dd x86_CallInterrupt_Interrupt7
	dd x86_CallInterrupt_Interrupt8
	dd x86_CallInterrupt_Interrupt9
	dd x86_CallInterrupt_Interrupt10
	dd x86_CallInterrupt_Interrupt11
	dd x86_CallInterrupt_Interrupt12
	dd x86_CallInterrupt_Interrupt13
	dd x86_CallInterrupt_Interrupt14
	dd x86_CallInterrupt_Interrupt15
	dd x86_CallInterrupt_Interrupt16
	dd x86_CallInterrupt_Interrupt17
	dd x86_CallInterrupt_Interrupt18
	dd x86_CallInterrupt_Interrupt19
	dd x86_CallInterrupt_Interrupt20
	dd x86_CallInterrupt_Interrupt21
	dd x86_CallInterrupt_Interrupt22
	dd x86_CallInterrupt_Interrupt23
	dd x86_CallInterrupt_Interrupt24
	dd x86_CallInterrupt_Interrupt25
	dd x86_CallInterrupt_Interrupt26
	dd x86_CallInterrupt_Interrupt27
	dd x86_CallInterrupt_Interrupt28
	dd x86_CallInterrupt_Interrupt29
	dd x86_CallInterrupt_Interrupt30
	dd x86_CallInterrupt_Interrupt31
	dd x86_CallInterrupt_Interrupt32
	dd x86_CallInterrupt_Interrupt33
	dd x86_CallInterrupt_Interrupt34
	dd x86_CallInterrupt_Interrupt35
	dd x86_CallInterrupt_Interrupt36
	dd x86_CallInterrupt_Interrupt37
	dd x86_CallInterrupt_Interrupt38
	dd x86_CallInterrupt_Interrupt39
	dd x86_CallInterrupt_Interrupt40
	dd x86_CallInterrupt_Interrupt41
	dd x86_CallInterrupt_Interrupt42
	dd x86_CallInterrupt_Interrupt43
	dd x86_CallInterrupt_Interrupt44
	dd x86_CallInterrupt_Interrupt45
	dd x86_CallInterrupt_Interrupt46
	dd x86_CallInterrupt_Interrupt47
	dd x86_CallInterrupt_Interrupt48
	dd x86_CallInterrupt_Interrupt49
	dd x86_CallInterrupt_Interrupt50
	dd x86_CallInterrupt_Interrupt51
	dd x86_CallInterrupt_Interrupt52
	dd x86_CallInterrupt_Interrupt53
	dd x86_CallInterrupt_Interrupt54
	dd x86_CallInterrupt_Interrupt55
	dd x86_CallInterrupt_Interrupt56
	dd x86_CallInterrupt_Interrupt57
	dd x86_CallInterrupt_Interrupt58
	dd x86_CallInterrupt_Interrupt59
	dd x86_CallInterrupt_Interrupt60
	dd x86_CallInterrupt_Interrupt61
	dd x86_CallInterrupt_Interrupt62
	dd x86_CallInterrupt_Interrupt63
	dd x86_CallInterrupt_Interrupt64
	dd x86_CallInterrupt_Interrupt65
	dd x86_CallInterrupt_Interrupt66
	dd x86_CallInterrupt_Interrupt67
	dd x86_CallInterrupt_Interrupt68
	dd x86_CallInterrupt_Interrupt69
	dd x86_CallInterrupt_Interrupt70
	dd x86_CallInterrupt_Interrupt71
	dd x86_CallInterrupt_Interrupt72
	dd x86_CallInterrupt_Interrupt73
	dd x86_CallInterrupt_Interrupt74
	dd x86_CallInterrupt_Interrupt75
	dd x86_CallInterrupt_Interrupt76
	dd x86_CallInterrupt_Interrupt77
	dd x86_CallInterrupt_Interrupt78
	dd x86_CallInterrupt_Interrupt79
	dd x86_CallInterrupt_Interrupt80
	dd x86_CallInterrupt_Interrupt81
	dd x86_CallInterrupt_Interrupt82
	dd x86_CallInterrupt_Interrupt83
	dd x86_CallInterrupt_Interrupt84
	dd x86_CallInterrupt_Interrupt85
	dd x86_CallInterrupt_Interrupt86
	dd x86_CallInterrupt_Interrupt87
	dd x86_CallInterrupt_Interrupt88
	dd x86_CallInterrupt_Interrupt89
	dd x86_CallInterrupt_Interrupt90
	dd x86_CallInterrupt_Interrupt91
	dd x86_CallInterrupt_Interrupt92
	dd x86_CallInterrupt_Interrupt93
	dd x86_CallInterrupt_Interrupt94
	dd x86_CallInterrupt_Interrupt95
	dd x86_CallInterrupt_Interrupt96
	dd x86_CallInterrupt_Interrupt97
	dd x86_CallInterrupt_Interrupt98
	dd x86_CallInterrupt_Interrupt99
	dd x86_CallInterrupt_Interrupt100
	dd x86_CallInterrupt_Interrupt101
	dd x86_CallInterrupt_Interrupt102
	dd x86_CallInterrupt_Interrupt103
	dd x86_CallInterrupt_Interrupt104
	dd x86_CallInterrupt_Interrupt105
	dd x86_CallInterrupt_Interrupt106
	dd x86_CallInterrupt_Interrupt107
	dd x86_CallInterrupt_Interrupt108
	dd x86_CallInterrupt_Interrupt109
	dd x86_CallInterrupt_Interrupt110
	dd x86_CallInterrupt_Interrupt111
	dd x86_CallInterrupt_Interrupt112
	dd x86_CallInterrupt_Interrupt113
	dd x86_CallInterrupt_Interrupt114
	dd x86_CallInterrupt_Interrupt115
	dd x86_CallInterrupt_Interrupt116
	dd x86_CallInterrupt_Interrupt117
	dd x86_CallInterrupt_Interrupt118
	dd x86_CallInterrupt_Interrupt119
	dd x86_CallInterrupt_Interrupt120
	dd x86_CallInterrupt_Interrupt121
	dd x86_CallInterrupt_Interrupt122
	dd x86_CallInterrupt_Interrupt123
	dd x86_CallInterrupt_Interrupt124
	dd x86_CallInterrupt_Interrupt125
	dd x86_CallInterrupt_Interrupt126
	dd x86_CallInterrupt_Interrupt127
	dd x86_CallInterrupt_Interrupt128
	dd x86_CallInterrupt_Interrupt129
	dd x86_CallInterrupt_Interrupt130
	dd x86_CallInterrupt_Interrupt131
	dd x86_CallInterrupt_Interrupt132
	dd x86_CallInterrupt_Interrupt133
	dd x86_CallInterrupt_Interrupt134
	dd x86_CallInterrupt_Interrupt135
	dd x86_CallInterrupt_Interrupt136
	dd x86_CallInterrupt_Interrupt137
	dd x86_CallInterrupt_Interrupt138
	dd x86_CallInterrupt_Interrupt139
	dd x86_CallInterrupt_Interrupt140
	dd x86_CallInterrupt_Interrupt141
	dd x86_CallInterrupt_Interrupt142
	dd x86_CallInterrupt_Interrupt143
	dd x86_CallInterrupt_Interrupt144
	dd x86_CallInterrupt_Interrupt145
	dd x86_CallInterrupt_Interrupt146
	dd x86_CallInterrupt_Interrupt147
	dd x86_CallInterrupt_Interrupt148
	dd x86_CallInterrupt_Interrupt149
	dd x86_CallInterrupt_Interrupt150
	dd x86_CallInterrupt_Interrupt151
	dd x86_CallInterrupt_Interrupt152
	dd x86_CallInterrupt_Interrupt153
	dd x86_CallInterrupt_Interrupt154
	dd x86_CallInterrupt_Interrupt155
	dd x86_CallInterrupt_Interrupt156
	dd x86_CallInterrupt_Interrupt157
	dd x86_CallInterrupt_Interrupt158
	dd x86_CallInterrupt_Interrupt159
	dd x86_CallInterrupt_Interrupt160
	dd x86_CallInterrupt_Interrupt161
	dd x86_CallInterrupt_Interrupt162
	dd x86_CallInterrupt_Interrupt163
	dd x86_CallInterrupt_Interrupt164
	dd x86_CallInterrupt_Interrupt165
	dd x86_CallInterrupt_Interrupt166
	dd x86_CallInterrupt_Interrupt167
	dd x86_CallInterrupt_Interrupt168
	dd x86_CallInterrupt_Interrupt169
	dd x86_CallInterrupt_Interrupt170
	dd x86_CallInterrupt_Interrupt171
	dd x86_CallInterrupt_Interrupt172
	dd x86_CallInterrupt_Interrupt173
	dd x86_CallInterrupt_Interrupt174
	dd x86_CallInterrupt_Interrupt175
	dd x86_CallInterrupt_Interrupt176
	dd x86_CallInterrupt_Interrupt177
	dd x86_CallInterrupt_Interrupt178
	dd x86_CallInterrupt_Interrupt179
	dd x86_CallInterrupt_Interrupt180
	dd x86_CallInterrupt_Interrupt181
	dd x86_CallInterrupt_Interrupt182
	dd x86_CallInterrupt_Interrupt183
	dd x86_CallInterrupt_Interrupt184
	dd x86_CallInterrupt_Interrupt185
	dd x86_CallInterrupt_Interrupt186
	dd x86_CallInterrupt_Interrupt187
	dd x86_CallInterrupt_Interrupt188
	dd x86_CallInterrupt_Interrupt189
	dd x86_CallInterrupt_Interrupt190
	dd x86_CallInterrupt_Interrupt191
	dd x86_CallInterrupt_Interrupt192
	dd x86_CallInterrupt_Interrupt193
	dd x86_CallInterrupt_Interrupt194
	dd x86_CallInterrupt_Interrupt195
	dd x86_CallInterrupt_Interrupt196
	dd x86_CallInterrupt_Interrupt197
	dd x86_CallInterrupt_Interrupt198
	dd x86_CallInterrupt_Interrupt199
	dd x86_CallInterrupt_Interrupt200
	dd x86_CallInterrupt_Interrupt201
	dd x86_CallInterrupt_Interrupt202
	dd x86_CallInterrupt_Interrupt203
	dd x86_CallInterrupt_Interrupt204
	dd x86_CallInterrupt_Interrupt205
	dd x86_CallInterrupt_Interrupt206
	dd x86_CallInterrupt_Interrupt207
	dd x86_CallInterrupt_Interrupt208
	dd x86_CallInterrupt_Interrupt209
	dd x86_CallInterrupt_Interrupt210
	dd x86_CallInterrupt_Interrupt211
	dd x86_CallInterrupt_Interrupt212
	dd x86_CallInterrupt_Interrupt213
	dd x86_CallInterrupt_Interrupt214
	dd x86_CallInterrupt_Interrupt215
	dd x86_CallInterrupt_Interrupt216
	dd x86_CallInterrupt_Interrupt217
	dd x86_CallInterrupt_Interrupt218
	dd x86_CallInterrupt_Interrupt219
	dd x86_CallInterrupt_Interrupt220
	dd x86_CallInterrupt_Interrupt221
	dd x86_CallInterrupt_Interrupt222
	dd x86_CallInterrupt_Interrupt223
	dd x86_CallInterrupt_Interrupt224
	dd x86_CallInterrupt_Interrupt225
	dd x86_CallInterrupt_Interrupt226
	dd x86_CallInterrupt_Interrupt227
	dd x86_CallInterrupt_Interrupt228
	dd x86_CallInterrupt_Interrupt229
	dd x86_CallInterrupt_Interrupt230
	dd x86_CallInterrupt_Interrupt231
	dd x86_CallInterrupt_Interrupt232
	dd x86_CallInterrupt_Interrupt233
	dd x86_CallInterrupt_Interrupt234
	dd x86_CallInterrupt_Interrupt235
	dd x86_CallInterrupt_Interrupt236
	dd x86_CallInterrupt_Interrupt237
	dd x86_CallInterrupt_Interrupt238
	dd x86_CallInterrupt_Interrupt239
	dd x86_CallInterrupt_Interrupt240
	dd x86_CallInterrupt_Interrupt241
	dd x86_CallInterrupt_Interrupt242
	dd x86_CallInterrupt_Interrupt243
	dd x86_CallInterrupt_Interrupt244
	dd x86_CallInterrupt_Interrupt245
	dd x86_CallInterrupt_Interrupt246
	dd x86_CallInterrupt_Interrupt247
	dd x86_CallInterrupt_Interrupt248
	dd x86_CallInterrupt_Interrupt249
	dd x86_CallInterrupt_Interrupt250
	dd x86_CallInterrupt_Interrupt251
	dd x86_CallInterrupt_Interrupt252
	dd x86_CallInterrupt_Interrupt253
	dd x86_CallInterrupt_Interrupt254
	dd x86_CallInterrupt_Interrupt255
; END x86_CallInterrupt_JumpTable

section .text

; x86_CallInterrupt Interrupts calls
	x86_CallInterrupt_Interrupt0:
		int 0x0
		jmp after
	x86_CallInterrupt_Interrupt1:
		int 0x1
		jmp after
	x86_CallInterrupt_Interrupt2:
		int 0x2
		jmp after
	x86_CallInterrupt_Interrupt3:
		int 0x3
		jmp after
	x86_CallInterrupt_Interrupt4:
		int 0x4
		jmp after
	x86_CallInterrupt_Interrupt5:
		int 0x5
		jmp after
	x86_CallInterrupt_Interrupt6:
		int 0x6
		jmp after
	x86_CallInterrupt_Interrupt7:
		int 0x7
		jmp after
	x86_CallInterrupt_Interrupt8:
		int 0x8
		jmp after
	x86_CallInterrupt_Interrupt9:
		int 0x9
		jmp after
	x86_CallInterrupt_Interrupt10:
		int 0xa
		jmp after
	x86_CallInterrupt_Interrupt11:
		int 0xb
		jmp after
	x86_CallInterrupt_Interrupt12:
		int 0xc
		jmp after
	x86_CallInterrupt_Interrupt13:
		int 0xd
		jmp after
	x86_CallInterrupt_Interrupt14:
		int 0xe
		jmp after
	x86_CallInterrupt_Interrupt15:
		int 0xf
		jmp after
	x86_CallInterrupt_Interrupt16:
		int 0x10
		jmp after
	x86_CallInterrupt_Interrupt17:
		int 0x11
		jmp after
	x86_CallInterrupt_Interrupt18:
		int 0x12
		jmp after
	x86_CallInterrupt_Interrupt19:
		int 0x13
		jmp after
	x86_CallInterrupt_Interrupt20:
		int 0x14
		jmp after
	x86_CallInterrupt_Interrupt21:
		int 0x15
		jmp after
	x86_CallInterrupt_Interrupt22:
		int 0x16
		jmp after
	x86_CallInterrupt_Interrupt23:
		int 0x17
		jmp after
	x86_CallInterrupt_Interrupt24:
		int 0x18
		jmp after
	x86_CallInterrupt_Interrupt25:
		int 0x19
		jmp after
	x86_CallInterrupt_Interrupt26:
		int 0x1a
		jmp after
	x86_CallInterrupt_Interrupt27:
		int 0x1b
		jmp after
	x86_CallInterrupt_Interrupt28:
		int 0x1c
		jmp after
	x86_CallInterrupt_Interrupt29:
		int 0x1d
		jmp after
	x86_CallInterrupt_Interrupt30:
		int 0x1e
		jmp after
	x86_CallInterrupt_Interrupt31:
		int 0x1f
		jmp after
	x86_CallInterrupt_Interrupt32:
		int 0x20
		jmp after
	x86_CallInterrupt_Interrupt33:
		int 0x21
		jmp after
	x86_CallInterrupt_Interrupt34:
		int 0x22
		jmp after
	x86_CallInterrupt_Interrupt35:
		int 0x23
		jmp after
	x86_CallInterrupt_Interrupt36:
		int 0x24
		jmp after
	x86_CallInterrupt_Interrupt37:
		int 0x25
		jmp after
	x86_CallInterrupt_Interrupt38:
		int 0x26
		jmp after
	x86_CallInterrupt_Interrupt39:
		int 0x27
		jmp after
	x86_CallInterrupt_Interrupt40:
		int 0x28
		jmp after
	x86_CallInterrupt_Interrupt41:
		int 0x29
		jmp after
	x86_CallInterrupt_Interrupt42:
		int 0x2a
		jmp after
	x86_CallInterrupt_Interrupt43:
		int 0x2b
		jmp after
	x86_CallInterrupt_Interrupt44:
		int 0x2c
		jmp after
	x86_CallInterrupt_Interrupt45:
		int 0x2d
		jmp after
	x86_CallInterrupt_Interrupt46:
		int 0x2e
		jmp after
	x86_CallInterrupt_Interrupt47:
		int 0x2f
		jmp after
	x86_CallInterrupt_Interrupt48:
		int 0x30
		jmp after
	x86_CallInterrupt_Interrupt49:
		int 0x31
		jmp after
	x86_CallInterrupt_Interrupt50:
		int 0x32
		jmp after
	x86_CallInterrupt_Interrupt51:
		int 0x33
		jmp after
	x86_CallInterrupt_Interrupt52:
		int 0x34
		jmp after
	x86_CallInterrupt_Interrupt53:
		int 0x35
		jmp after
	x86_CallInterrupt_Interrupt54:
		int 0x36
		jmp after
	x86_CallInterrupt_Interrupt55:
		int 0x37
		jmp after
	x86_CallInterrupt_Interrupt56:
		int 0x38
		jmp after
	x86_CallInterrupt_Interrupt57:
		int 0x39
		jmp after
	x86_CallInterrupt_Interrupt58:
		int 0x3a
		jmp after
	x86_CallInterrupt_Interrupt59:
		int 0x3b
		jmp after
	x86_CallInterrupt_Interrupt60:
		int 0x3c
		jmp after
	x86_CallInterrupt_Interrupt61:
		int 0x3d
		jmp after
	x86_CallInterrupt_Interrupt62:
		int 0x3e
		jmp after
	x86_CallInterrupt_Interrupt63:
		int 0x3f
		jmp after
	x86_CallInterrupt_Interrupt64:
		int 0x40
		jmp after
	x86_CallInterrupt_Interrupt65:
		int 0x41
		jmp after
	x86_CallInterrupt_Interrupt66:
		int 0x42
		jmp after
	x86_CallInterrupt_Interrupt67:
		int 0x43
		jmp after
	x86_CallInterrupt_Interrupt68:
		int 0x44
		jmp after
	x86_CallInterrupt_Interrupt69:
		int 0x45
		jmp after
	x86_CallInterrupt_Interrupt70:
		int 0x46
		jmp after
	x86_CallInterrupt_Interrupt71:
		int 0x47
		jmp after
	x86_CallInterrupt_Interrupt72:
		int 0x48
		jmp after
	x86_CallInterrupt_Interrupt73:
		int 0x49
		jmp after
	x86_CallInterrupt_Interrupt74:
		int 0x4a
		jmp after
	x86_CallInterrupt_Interrupt75:
		int 0x4b
		jmp after
	x86_CallInterrupt_Interrupt76:
		int 0x4c
		jmp after
	x86_CallInterrupt_Interrupt77:
		int 0x4d
		jmp after
	x86_CallInterrupt_Interrupt78:
		int 0x4e
		jmp after
	x86_CallInterrupt_Interrupt79:
		int 0x4f
		jmp after
	x86_CallInterrupt_Interrupt80:
		int 0x50
		jmp after
	x86_CallInterrupt_Interrupt81:
		int 0x51
		jmp after
	x86_CallInterrupt_Interrupt82:
		int 0x52
		jmp after
	x86_CallInterrupt_Interrupt83:
		int 0x53
		jmp after
	x86_CallInterrupt_Interrupt84:
		int 0x54
		jmp after
	x86_CallInterrupt_Interrupt85:
		int 0x55
		jmp after
	x86_CallInterrupt_Interrupt86:
		int 0x56
		jmp after
	x86_CallInterrupt_Interrupt87:
		int 0x57
		jmp after
	x86_CallInterrupt_Interrupt88:
		int 0x58
		jmp after
	x86_CallInterrupt_Interrupt89:
		int 0x59
		jmp after
	x86_CallInterrupt_Interrupt90:
		int 0x5a
		jmp after
	x86_CallInterrupt_Interrupt91:
		int 0x5b
		jmp after
	x86_CallInterrupt_Interrupt92:
		int 0x5c
		jmp after
	x86_CallInterrupt_Interrupt93:
		int 0x5d
		jmp after
	x86_CallInterrupt_Interrupt94:
		int 0x5e
		jmp after
	x86_CallInterrupt_Interrupt95:
		int 0x5f
		jmp after
	x86_CallInterrupt_Interrupt96:
		int 0x60
		jmp after
	x86_CallInterrupt_Interrupt97:
		int 0x61
		jmp after
	x86_CallInterrupt_Interrupt98:
		int 0x62
		jmp after
	x86_CallInterrupt_Interrupt99:
		int 0x63
		jmp after
	x86_CallInterrupt_Interrupt100:
		int 0x64
		jmp after
	x86_CallInterrupt_Interrupt101:
		int 0x65
		jmp after
	x86_CallInterrupt_Interrupt102:
		int 0x66
		jmp after
	x86_CallInterrupt_Interrupt103:
		int 0x67
		jmp after
	x86_CallInterrupt_Interrupt104:
		int 0x68
		jmp after
	x86_CallInterrupt_Interrupt105:
		int 0x69
		jmp after
	x86_CallInterrupt_Interrupt106:
		int 0x6a
		jmp after
	x86_CallInterrupt_Interrupt107:
		int 0x6b
		jmp after
	x86_CallInterrupt_Interrupt108:
		int 0x6c
		jmp after
	x86_CallInterrupt_Interrupt109:
		int 0x6d
		jmp after
	x86_CallInterrupt_Interrupt110:
		int 0x6e
		jmp after
	x86_CallInterrupt_Interrupt111:
		int 0x6f
		jmp after
	x86_CallInterrupt_Interrupt112:
		int 0x70
		jmp after
	x86_CallInterrupt_Interrupt113:
		int 0x71
		jmp after
	x86_CallInterrupt_Interrupt114:
		int 0x72
		jmp after
	x86_CallInterrupt_Interrupt115:
		int 0x73
		jmp after
	x86_CallInterrupt_Interrupt116:
		int 0x74
		jmp after
	x86_CallInterrupt_Interrupt117:
		int 0x75
		jmp after
	x86_CallInterrupt_Interrupt118:
		int 0x76
		jmp after
	x86_CallInterrupt_Interrupt119:
		int 0x77
		jmp after
	x86_CallInterrupt_Interrupt120:
		int 0x78
		jmp after
	x86_CallInterrupt_Interrupt121:
		int 0x79
		jmp after
	x86_CallInterrupt_Interrupt122:
		int 0x7a
		jmp after
	x86_CallInterrupt_Interrupt123:
		int 0x7b
		jmp after
	x86_CallInterrupt_Interrupt124:
		int 0x7c
		jmp after
	x86_CallInterrupt_Interrupt125:
		int 0x7d
		jmp after
	x86_CallInterrupt_Interrupt126:
		int 0x7e
		jmp after
	x86_CallInterrupt_Interrupt127:
		int 0x7f
		jmp after
	x86_CallInterrupt_Interrupt128:
		int 0x80
		jmp after
	x86_CallInterrupt_Interrupt129:
		int 0x81
		jmp after
	x86_CallInterrupt_Interrupt130:
		int 0x82
		jmp after
	x86_CallInterrupt_Interrupt131:
		int 0x83
		jmp after
	x86_CallInterrupt_Interrupt132:
		int 0x84
		jmp after
	x86_CallInterrupt_Interrupt133:
		int 0x85
		jmp after
	x86_CallInterrupt_Interrupt134:
		int 0x86
		jmp after
	x86_CallInterrupt_Interrupt135:
		int 0x87
		jmp after
	x86_CallInterrupt_Interrupt136:
		int 0x88
		jmp after
	x86_CallInterrupt_Interrupt137:
		int 0x89
		jmp after
	x86_CallInterrupt_Interrupt138:
		int 0x8a
		jmp after
	x86_CallInterrupt_Interrupt139:
		int 0x8b
		jmp after
	x86_CallInterrupt_Interrupt140:
		int 0x8c
		jmp after
	x86_CallInterrupt_Interrupt141:
		int 0x8d
		jmp after
	x86_CallInterrupt_Interrupt142:
		int 0x8e
		jmp after
	x86_CallInterrupt_Interrupt143:
		int 0x8f
		jmp after
	x86_CallInterrupt_Interrupt144:
		int 0x90
		jmp after
	x86_CallInterrupt_Interrupt145:
		int 0x91
		jmp after
	x86_CallInterrupt_Interrupt146:
		int 0x92
		jmp after
	x86_CallInterrupt_Interrupt147:
		int 0x93
		jmp after
	x86_CallInterrupt_Interrupt148:
		int 0x94
		jmp after
	x86_CallInterrupt_Interrupt149:
		int 0x95
		jmp after
	x86_CallInterrupt_Interrupt150:
		int 0x96
		jmp after
	x86_CallInterrupt_Interrupt151:
		int 0x97
		jmp after
	x86_CallInterrupt_Interrupt152:
		int 0x98
		jmp after
	x86_CallInterrupt_Interrupt153:
		int 0x99
		jmp after
	x86_CallInterrupt_Interrupt154:
		int 0x9a
		jmp after
	x86_CallInterrupt_Interrupt155:
		int 0x9b
		jmp after
	x86_CallInterrupt_Interrupt156:
		int 0x9c
		jmp after
	x86_CallInterrupt_Interrupt157:
		int 0x9d
		jmp after
	x86_CallInterrupt_Interrupt158:
		int 0x9e
		jmp after
	x86_CallInterrupt_Interrupt159:
		int 0x9f
		jmp after
	x86_CallInterrupt_Interrupt160:
		int 0xa0
		jmp after
	x86_CallInterrupt_Interrupt161:
		int 0xa1
		jmp after
	x86_CallInterrupt_Interrupt162:
		int 0xa2
		jmp after
	x86_CallInterrupt_Interrupt163:
		int 0xa3
		jmp after
	x86_CallInterrupt_Interrupt164:
		int 0xa4
		jmp after
	x86_CallInterrupt_Interrupt165:
		int 0xa5
		jmp after
	x86_CallInterrupt_Interrupt166:
		int 0xa6
		jmp after
	x86_CallInterrupt_Interrupt167:
		int 0xa7
		jmp after
	x86_CallInterrupt_Interrupt168:
		int 0xa8
		jmp after
	x86_CallInterrupt_Interrupt169:
		int 0xa9
		jmp after
	x86_CallInterrupt_Interrupt170:
		int 0xaa
		jmp after
	x86_CallInterrupt_Interrupt171:
		int 0xab
		jmp after
	x86_CallInterrupt_Interrupt172:
		int 0xac
		jmp after
	x86_CallInterrupt_Interrupt173:
		int 0xad
		jmp after
	x86_CallInterrupt_Interrupt174:
		int 0xae
		jmp after
	x86_CallInterrupt_Interrupt175:
		int 0xaf
		jmp after
	x86_CallInterrupt_Interrupt176:
		int 0xb0
		jmp after
	x86_CallInterrupt_Interrupt177:
		int 0xb1
		jmp after
	x86_CallInterrupt_Interrupt178:
		int 0xb2
		jmp after
	x86_CallInterrupt_Interrupt179:
		int 0xb3
		jmp after
	x86_CallInterrupt_Interrupt180:
		int 0xb4
		jmp after
	x86_CallInterrupt_Interrupt181:
		int 0xb5
		jmp after
	x86_CallInterrupt_Interrupt182:
		int 0xb6
		jmp after
	x86_CallInterrupt_Interrupt183:
		int 0xb7
		jmp after
	x86_CallInterrupt_Interrupt184:
		int 0xb8
		jmp after
	x86_CallInterrupt_Interrupt185:
		int 0xb9
		jmp after
	x86_CallInterrupt_Interrupt186:
		int 0xba
		jmp after
	x86_CallInterrupt_Interrupt187:
		int 0xbb
		jmp after
	x86_CallInterrupt_Interrupt188:
		int 0xbc
		jmp after
	x86_CallInterrupt_Interrupt189:
		int 0xbd
		jmp after
	x86_CallInterrupt_Interrupt190:
		int 0xbe
		jmp after
	x86_CallInterrupt_Interrupt191:
		int 0xbf
		jmp after
	x86_CallInterrupt_Interrupt192:
		int 0xc0
		jmp after
	x86_CallInterrupt_Interrupt193:
		int 0xc1
		jmp after
	x86_CallInterrupt_Interrupt194:
		int 0xc2
		jmp after
	x86_CallInterrupt_Interrupt195:
		int 0xc3
		jmp after
	x86_CallInterrupt_Interrupt196:
		int 0xc4
		jmp after
	x86_CallInterrupt_Interrupt197:
		int 0xc5
		jmp after
	x86_CallInterrupt_Interrupt198:
		int 0xc6
		jmp after
	x86_CallInterrupt_Interrupt199:
		int 0xc7
		jmp after
	x86_CallInterrupt_Interrupt200:
		int 0xc8
		jmp after
	x86_CallInterrupt_Interrupt201:
		int 0xc9
		jmp after
	x86_CallInterrupt_Interrupt202:
		int 0xca
		jmp after
	x86_CallInterrupt_Interrupt203:
		int 0xcb
		jmp after
	x86_CallInterrupt_Interrupt204:
		int 0xcc
		jmp after
	x86_CallInterrupt_Interrupt205:
		int 0xcd
		jmp after
	x86_CallInterrupt_Interrupt206:
		int 0xce
		jmp after
	x86_CallInterrupt_Interrupt207:
		int 0xcf
		jmp after
	x86_CallInterrupt_Interrupt208:
		int 0xd0
		jmp after
	x86_CallInterrupt_Interrupt209:
		int 0xd1
		jmp after
	x86_CallInterrupt_Interrupt210:
		int 0xd2
		jmp after
	x86_CallInterrupt_Interrupt211:
		int 0xd3
		jmp after
	x86_CallInterrupt_Interrupt212:
		int 0xd4
		jmp after
	x86_CallInterrupt_Interrupt213:
		int 0xd5
		jmp after
	x86_CallInterrupt_Interrupt214:
		int 0xd6
		jmp after
	x86_CallInterrupt_Interrupt215:
		int 0xd7
		jmp after
	x86_CallInterrupt_Interrupt216:
		int 0xd8
		jmp after
	x86_CallInterrupt_Interrupt217:
		int 0xd9
		jmp after
	x86_CallInterrupt_Interrupt218:
		int 0xda
		jmp after
	x86_CallInterrupt_Interrupt219:
		int 0xdb
		jmp after
	x86_CallInterrupt_Interrupt220:
		int 0xdc
		jmp after
	x86_CallInterrupt_Interrupt221:
		int 0xdd
		jmp after
	x86_CallInterrupt_Interrupt222:
		int 0xde
		jmp after
	x86_CallInterrupt_Interrupt223:
		int 0xdf
		jmp after
	x86_CallInterrupt_Interrupt224:
		int 0xe0
		jmp after
	x86_CallInterrupt_Interrupt225:
		int 0xe1
		jmp after
	x86_CallInterrupt_Interrupt226:
		int 0xe2
		jmp after
	x86_CallInterrupt_Interrupt227:
		int 0xe3
		jmp after
	x86_CallInterrupt_Interrupt228:
		int 0xe4
		jmp after
	x86_CallInterrupt_Interrupt229:
		int 0xe5
		jmp after
	x86_CallInterrupt_Interrupt230:
		int 0xe6
		jmp after
	x86_CallInterrupt_Interrupt231:
		int 0xe7
		jmp after
	x86_CallInterrupt_Interrupt232:
		int 0xe8
		jmp after
	x86_CallInterrupt_Interrupt233:
		int 0xe9
		jmp after
	x86_CallInterrupt_Interrupt234:
		int 0xea
		jmp after
	x86_CallInterrupt_Interrupt235:
		int 0xeb
		jmp after
	x86_CallInterrupt_Interrupt236:
		int 0xec
		jmp after
	x86_CallInterrupt_Interrupt237:
		int 0xed
		jmp after
	x86_CallInterrupt_Interrupt238:
		int 0xee
		jmp after
	x86_CallInterrupt_Interrupt239:
		int 0xef
		jmp after
	x86_CallInterrupt_Interrupt240:
		int 0xf0
		jmp after
	x86_CallInterrupt_Interrupt241:
		int 0xf1
		jmp after
	x86_CallInterrupt_Interrupt242:
		int 0xf2
		jmp after
	x86_CallInterrupt_Interrupt243:
		int 0xf3
		jmp after
	x86_CallInterrupt_Interrupt244:
		int 0xf4
		jmp after
	x86_CallInterrupt_Interrupt245:
		int 0xf5
		jmp after
	x86_CallInterrupt_Interrupt246:
		int 0xf6
		jmp after
	x86_CallInterrupt_Interrupt247:
		int 0xf7
		jmp after
	x86_CallInterrupt_Interrupt248:
		int 0xf8
		jmp after
	x86_CallInterrupt_Interrupt249:
		int 0xf9
		jmp after
	x86_CallInterrupt_Interrupt250:
		int 0xfa
		jmp after
	x86_CallInterrupt_Interrupt251:
		int 0xfb
		jmp after
	x86_CallInterrupt_Interrupt252:
		int 0xfc
		jmp after
	x86_CallInterrupt_Interrupt253:
		int 0xfd
		jmp after
	x86_CallInterrupt_Interrupt254:
		int 0xfe
		jmp after
	x86_CallInterrupt_Interrupt255:
		int 0xff
		jmp after
; end interrupts
