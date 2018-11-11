:main
    accept sfd ! ipaddr !
    recvreq not not
    sfd @ waithdr
    checkip &
    dup if answer
    not if deny
    main ;

:answer
    200ok
    $initialise_state
    absorbip
    squeezeip contact drop
    squeezeip contact drop
    decipher
    sfd @ close
    bye
    ;

:deny
    403forbidden
    sfd @ close
    ;

:decipher 0 decipher_loop drop ;
:decipher_loop
    dup ciphertext + c@ $drip ^ sfd @ write
    1 + dup 32 < if decipher_loop ;

:contact
    lvl c@ 0 = if ;
    dup 8000 connect cfd !
    cfd @ negative? if ;
    sendreq
    cfd @ waithdr
    mixresponse
    cfd @ close
    ;

:mixresponse 0 mixresponse_loop drop ;
:mixresponse_loop
    dup ciphertext + dup c@ cfd @ read ^ swap c!
    1 + dup 32 < if mixresponse_loop ;

:absorbip
    ipaddr @ dup 255 & swap 8 >>
             dup 255 & swap 8 >>
             dup 255 & swap 8 >>
    $absorb_byte
    $absorb_byte
    $absorb_byte
    $absorb_byte
    ;

:squeezeip
    2560
    $drip + 256 *
    $drip + 256 *
    $drip +
    ;

:checkip 10 ipaddr @ 24 >> =

:waithdr waithdr_noncr waithdr_loop drop ;
:waithdr_loop
    dup read
    dup dup 10 = dup    if waithdr_lf
    not swap 13 = not & if waithdr_noncr
    negative?           if waithdr_closed
    #nl c@ 2 <          if waithdr_loop
    ;
:waithdr_lf     #nl c@ 1 + #nl c! ;
:waithdr_noncr  0 #nl c! ;
:waithdr_closed 2 #nl c! ;

:recvreq
    2 lvl c!
    0
    sfd @ read 200 ^ 143 ^ if ;
    sfd @ read 182 ^ 243 ^ if ;
    sfd @ read 233 ^ 189 ^ if ;
    sfd @ read 229 ^ 197 ^ if ;
    sfd @ read 62 ^ 17 ^ if ;
    sfd @ read 0 ^ 103 ^ if ;
    sfd @ read 147 ^ 242 ^ if ;
    sfd @ read 205 ^ 185 ^ if ;
    sfd @ read 122 ^ 18 ^ if ;
    sfd @ read 38 ^ 67 ^ if ;
    sfd @ read 195 ^ 177 ^ if ;
    sfd @ read 211 ^ 140 ^ if ;
    sfd @ read 227 ^ 134 ^ if ;
    sfd @ read 120 ^ 14 ^ if ;
    sfd @ read 220 ^ 185 ^ if ;
    sfd @ read 211 ^ 161 ^ if ;
    sfd @ read 111 ^ 22 ^ if ;
    sfd @ read 115 ^ 28 ^ if ;
    sfd @ read 173 ^ 195 ^ if ;
    sfd @ read 165 ^ 192 ^ if ;
    sfd @ read 145 ^ 206 ^ if ;
    sfd @ read 232 ^ 156 ^ if ;
    sfd @ read 247 ^ 152 ^ if ;
    sfd @ read 248 ^ 167 ^ if ;
    sfd @ read 219 ^ 169 ^ if ;
    sfd @ read 73 ^ 44 ^ if ;
    sfd @ read 250 ^ 140 ^ if ;
    sfd @ read 42 ^ 79 ^ if ;
    sfd @ read 242 ^ 147 ^ if ;
    sfd @ read 102 ^ 10 ^ if ;
    sfd @ read 156 ^ 195 ^ if ;
    sfd @ read 101 ^ 10 ^ if ;
    sfd @ read 35 ^ 86 ^ if ;
    sfd @ read 70 ^ 52 ^ if ;
    sfd @ read 95 ^ 0 ^ if ;
    sfd @ read 248 ^ 139 ^ if ;
    sfd @ read 197 ^ 160 ^ if ;
    sfd @ read 80 ^ 51 ^ if ;
    sfd @ read 86 ^ 36 ^ if ;
    sfd @ read 48 ^ 85 ^ if ;
    sfd @ read 73 ^ 61 ^ if ;
    drop sfd @ read dup 32 = if ;
    0 swap '? ^ if ;
    sfd @ read '0 - lvl c!
    sfd @ read 32 ^ if ;
    2 lvl c@ < if ;
    not ;

:sendreq
    236 171 ^ cfd @ write
    210 151 ^ cfd @ write
    9 93 ^ cfd @ write
    74 106 ^ cfd @ write
    60 19 ^ cfd @ write
    30 121 ^ cfd @ write
    137 232 ^ cfd @ write
    71 51 ^ cfd @ write
    30 118 ^ cfd @ write
    53 80 ^ cfd @ write
    206 188 ^ cfd @ write
    178 237 ^ cfd @ write
    192 165 ^ cfd @ write
    148 226 ^ cfd @ write
    215 178 ^ cfd @ write
    67 49 ^ cfd @ write
    34 91 ^ cfd @ write
    1 110 ^ cfd @ write
    51 93 ^ cfd @ write
    173 200 ^ cfd @ write
    175 240 ^ cfd @ write
    130 246 ^ cfd @ write
    7 104 ^ cfd @ write
    145 206 ^ cfd @ write
    222 172 ^ cfd @ write
    19 118 ^ cfd @ write
    203 189 ^ cfd @ write
    114 23 ^ cfd @ write
    219 186 ^ cfd @ write
    234 134 ^ cfd @ write
    156 195 ^ cfd @ write
    44 67 ^ cfd @ write
    59 78 ^ cfd @ write
    162 208 ^ cfd @ write
    153 198 ^ cfd @ write
    24 107 ^ cfd @ write
    249 156 ^ cfd @ write
    247 148 ^ cfd @ write
    83 33 ^ cfd @ write
    25 124 ^ cfd @ write
    231 147 ^ cfd @ write
    240 207 ^ cfd @ write
    '0 lvl c@ + 1 - cfd @ write
    132 164 ^ cfd @ write
    81 25 ^ cfd @ write
    197 145 ^ cfd @ write
    154 206 ^ cfd @ write
    69 21 ^ cfd @ write
    47 0 ^ cfd @ write
    31 46 ^ cfd @ write
    182 152 ^ cfd @ write
    192 240 ^ cfd @ write
    42 39 ^ cfd @ write
    180 190 ^ cfd @ write
    130 143 ^ cfd @ write
    123 113 ^ cfd @ write
    ;

:403forbidden
    165 237 ^ sfd @ write
    174 250 ^ sfd @ write
    80 4 ^ sfd @ write
    60 108 ^ sfd @ write
    46 1 ^ sfd @ write
    188 141 ^ sfd @ write
    167 137 ^ sfd @ write
    93 109 ^ sfd @ write
    179 147 ^ sfd @ write
    231 211 ^ sfd @ write
    105 89 ^ sfd @ write
    180 135 ^ sfd @ write
    211 243 ^ sfd @ write
    142 200 ^ sfd @ write
    221 178 ^ sfd @ write
    137 251 ^ sfd @ write
    110 12 ^ sfd @ write
    52 93 ^ sfd @ write
    190 218 ^ sfd @ write
    57 93 ^ sfd @ write
    29 120 ^ sfd @ write
    102 8 ^ sfd @ write
    207 194 ^ sfd @ write
    212 222 ^ sfd @ write
    71 74 ^ sfd @ write
    154 144 ^ sfd @ write
    ;

:200ok
    115 59 ^ sfd @ write
    108 56 ^ sfd @ write
    121 45 ^ sfd @ write
    121 41 ^ sfd @ write
    82 125 ^ sfd @ write
    141 188 ^ sfd @ write
    10 36 ^ sfd @ write
    246 198 ^ sfd @ write
    21 53 ^ sfd @ write
    176 130 ^ sfd @ write
    60 12 ^ sfd @ write
    164 148 ^ sfd @ write
    146 178 ^ sfd @ write
    43 100 ^ sfd @ write
    37 110 ^ sfd @ write
    211 222 ^ sfd @ write
    220 214 ^ sfd @ write
    171 166 ^ sfd @ write
    79 69 ^ sfd @ write
    ;


(Util)
:range! over + 1 - swap range!loop drop drop ;
:range!loop
    1 - swap over over c!
    1 - swap dup if range!loop ;

:dump@ over + 1 - swap dump@loop drop drop ;
:dump@loop
    1 - swap over over c@ . drop drop
    1 - swap dup if dump@loop ;

:negative? 46340 46341 * 41707 + swap < ;
:not 0 = ;


(Spritz)
:$initialise_state
    0 dup $i c!
      dup $j c!
      dup $k c!
      dup $z c!
          $a c!
    1     $w c!
    256 $S range!
    ;

:$absorb_byte
    dup 15 & $absorb_nibble
        4 >> $absorb_nibble
    ;
:$absorb_nibble
    $a c@ 128 = if $shuffle
    128 +  $a c@  $swap
    $a c@ 1 + $a c!
    ;

:$drip
    $a c@ if $shuffle
    $update
    $output
    ;
:$output
    $k c@ $z c@ + 255 & $S + c@
          $i c@ + 255 & $S + c@
          $j c@ + 255 & $S + c@
    dup $z c!
    ;

:$shuffle
    512 $whip
    $crush
    512 $whip
    $crush
    512 $whip
    0 $a c!
    ;

:$whip
    $whip_loop drop
    $w c@ 2 + $w c!
    ;
:$whip_loop $update 1 - dup if $whip_loop ;
:$update
    $i c@ $w c@ + $i c!
    $i c@ $S + c@ $j c@ + 255 & $S + c@ $k c@ + $j c!
    $j c@ $S + c@  $k c@ + $i c@ + $k c!
    $i c@ $j c@ $swap
    ;

:$crush 128 $crush_loop drop ;
:$crush_loop 1 - $crush_body dup if $crush_loop ;
:$crush_body
    dup $S + c@
    over 255 swap - $S + c@
    swap < if $crush_swap
    ;
:$crush_swap
    dup
    dup 255 swap -
    $swap
    ;

:$swap
    $S + swap $S +
    dup  c@ $tmp c!
    over c@ swap c!
    $tmp c@ swap c!
    ;
