#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************

PRJ=..$/..$/..
PRJNAME=help_sbasic
PACKAGE = text/sbasic
TARGET  = text_sbasic
MODULE  = sbasic

LANGUAGE_FILELIST= guide/create_dialog.xhp \
guide/insert_control.xhp \
guide/sample_code.xhp \
guide/show_dialog.xhp \
guide/translation.xhp \
shared/00000002.xhp \
shared/00000003.xhp \
shared/01000000.xhp \
shared/01010210.xhp \
shared/01020000.xhp \
shared/01020100.xhp \
shared/01020200.xhp \
shared/01020300.xhp \
shared/01020500.xhp \
shared/01030000.xhp \
shared/01030100.xhp \
shared/01030200.xhp \
shared/01030300.xhp \
shared/01030400.xhp \
shared/01040000.xhp \
shared/01050000.xhp \
shared/01050100.xhp \
shared/01050200.xhp \
shared/01050300.xhp \
shared/01/06130000.xhp \
shared/01/06130100.xhp \
shared/01/06130500.xhp \
shared/01170100.xhp \
shared/01170101.xhp \
shared/01170103.xhp \
shared/02/11010000.xhp \
shared/02/11020000.xhp \
shared/02/11030000.xhp \
shared/02/11040000.xhp \
shared/02/11050000.xhp \
shared/02/11060000.xhp \
shared/02/11070000.xhp \
shared/02/11080000.xhp \
shared/02/11090000.xhp \
shared/02/11100000.xhp \
shared/02/11110000.xhp \
shared/02/11120000.xhp \
shared/02/11140000.xhp \
shared/02/11150000.xhp \
shared/02/11160000.xhp \
shared/02/11170000.xhp \
shared/02/11180000.xhp \
shared/02/11190000.xhp \
shared/02/20000000.xhp \
shared/03000000.xhp \
shared/03010000.xhp \
shared/03010100.xhp \
shared/03010101.xhp \
shared/03010102.xhp \
shared/03010103.xhp \
shared/03010200.xhp \
shared/03010201.xhp \
shared/03010300.xhp \
shared/03010301.xhp \
shared/03010302.xhp \
shared/03010303.xhp \
shared/03010304.xhp \
shared/03010305.xhp \
shared/03020000.xhp \
shared/03020100.xhp \
shared/03020101.xhp \
shared/03020102.xhp \
shared/03020103.xhp \
shared/03020104.xhp \
shared/03020200.xhp \
shared/03020201.xhp \
shared/03020202.xhp \
shared/03020203.xhp \
shared/03020204.xhp \
shared/03020205.xhp \
shared/03020301.xhp \
shared/03020302.xhp \
shared/03020303.xhp \
shared/03020304.xhp \
shared/03020305.xhp \
shared/03020400.xhp \
shared/03020401.xhp \
shared/03020402.xhp \
shared/03020403.xhp \
shared/03020404.xhp \
shared/03020405.xhp \
shared/03020406.xhp \
shared/03020407.xhp \
shared/03020408.xhp \
shared/03020409.xhp \
shared/03020410.xhp \
shared/03020411.xhp \
shared/03020412.xhp \
shared/03020413.xhp \
shared/03020414.xhp \
shared/03020415.xhp \
shared/03030000.xhp \
shared/03030100.xhp \
shared/03030101.xhp \
shared/03030102.xhp \
shared/03030103.xhp \
shared/03030104.xhp \
shared/03030105.xhp \
shared/03030106.xhp \
shared/03030107.xhp \
shared/03030108.xhp \
shared/03030110.xhp \
shared/03030120.xhp \
shared/03030130.xhp \
shared/03030200.xhp \
shared/03030201.xhp \
shared/03030202.xhp \
shared/03030203.xhp \
shared/03030204.xhp \
shared/03030205.xhp \
shared/03030206.xhp \
shared/03030300.xhp \
shared/03030301.xhp \
shared/03030302.xhp \
shared/03030303.xhp \
shared/03050000.xhp \
shared/03050100.xhp \
shared/03050200.xhp \
shared/03050300.xhp \
shared/03050500.xhp \
shared/03060000.xhp \
shared/03060100.xhp \
shared/03060200.xhp \
shared/03060300.xhp \
shared/03060400.xhp \
shared/03060500.xhp \
shared/03060600.xhp \
shared/03070000.xhp \
shared/03070100.xhp \
shared/03070200.xhp \
shared/03070300.xhp \
shared/03070400.xhp \
shared/03070500.xhp \
shared/03070600.xhp \
shared/03080000.xhp \
shared/03080100.xhp \
shared/03080101.xhp \
shared/03080102.xhp \
shared/03080103.xhp \
shared/03080104.xhp \
shared/03080200.xhp \
shared/03080201.xhp \
shared/03080202.xhp \
shared/03080300.xhp \
shared/03080301.xhp \
shared/03080302.xhp \
shared/03080400.xhp \
shared/03080401.xhp \
shared/03080500.xhp \
shared/03080501.xhp \
shared/03080502.xhp \
shared/03080600.xhp \
shared/03080601.xhp \
shared/03080700.xhp \
shared/03080701.xhp \
shared/03080800.xhp \
shared/03080801.xhp \
shared/03080802.xhp \
shared/03090000.xhp \
shared/03090100.xhp \
shared/03090101.xhp \
shared/03090102.xhp \
shared/03090103.xhp \
shared/03090200.xhp \
shared/03090201.xhp \
shared/03090202.xhp \
shared/03090203.xhp \
shared/03090300.xhp \
shared/03090301.xhp \
shared/03090302.xhp \
shared/03090303.xhp \
shared/03090400.xhp \
shared/03090401.xhp \
shared/03090402.xhp \
shared/03090403.xhp \
shared/03090404.xhp \
shared/03090405.xhp \
shared/03090406.xhp \
shared/03090407.xhp \
shared/03090408.xhp \
shared/03090409.xhp \
shared/03090410.xhp \
shared/03090411.xhp \
shared/03090412.xhp \
shared/03100000.xhp \
shared/03100050.xhp \
shared/03100060.xhp \
shared/03100070.xhp \
shared/03100080.xhp \
shared/03100100.xhp \
shared/03100300.xhp \
shared/03100400.xhp \
shared/03100500.xhp \
shared/03100600.xhp \
shared/03100700.xhp \
shared/03100900.xhp \
shared/03101000.xhp \
shared/03101100.xhp \
shared/03101110.xhp \
shared/03101120.xhp \
shared/03101130.xhp \
shared/03101140.xhp \
shared/03101300.xhp \
shared/03101400.xhp \
shared/03101500.xhp \
shared/03101600.xhp \
shared/03101700.xhp \
shared/03102000.xhp \
shared/03102100.xhp \
shared/03102101.xhp \
shared/03102200.xhp \
shared/03102300.xhp \
shared/03102400.xhp \
shared/03102450.xhp \
shared/03102600.xhp \
shared/03102700.xhp \
shared/03102800.xhp \
shared/03102900.xhp \
shared/03103000.xhp \
shared/03103100.xhp \
shared/03103200.xhp \
shared/03103300.xhp \
shared/03103400.xhp \
shared/03103450.xhp \
shared/03103500.xhp \
shared/03103600.xhp \
shared/03103700.xhp \
shared/03103800.xhp \
shared/03103900.xhp \
shared/03104000.xhp \
shared/03104100.xhp \
shared/03104200.xhp \
shared/03104300.xhp \
shared/03104400.xhp \
shared/03104500.xhp \
shared/03104600.xhp \
shared/03104700.xhp \
shared/03110000.xhp \
shared/03110100.xhp \
shared/03120000.xhp \
shared/03120100.xhp \
shared/03120101.xhp \
shared/03120102.xhp \
shared/03120103.xhp \
shared/03120104.xhp \
shared/03120105.xhp \
shared/03120200.xhp \
shared/03120201.xhp \
shared/03120202.xhp \
shared/03120300.xhp \
shared/03120301.xhp \
shared/03120302.xhp \
shared/03120303.xhp \
shared/03120304.xhp \
shared/03120305.xhp \
shared/03120306.xhp \
shared/03120307.xhp \
shared/03120308.xhp \
shared/03120309.xhp \
shared/03120310.xhp \
shared/03120311.xhp \
shared/03120312.xhp \
shared/03120313.xhp \
shared/03120314.xhp \
shared/03120315.xhp \
shared/03120400.xhp \
shared/03120401.xhp \
shared/03120402.xhp \
shared/03120403.xhp \
shared/03130000.xhp \
shared/03130100.xhp \
shared/03130500.xhp \
shared/03130600.xhp \
shared/03130700.xhp \
shared/03130800.xhp \
shared/03131000.xhp \
shared/03131300.xhp \
shared/03131400.xhp \
shared/03131500.xhp \
shared/03131600.xhp \
shared/03131700.xhp \
shared/03131800.xhp \
shared/03131900.xhp \
shared/03132000.xhp \
shared/03132100.xhp \
shared/03132200.xhp \
shared/03132300.xhp \
shared/03132400.xhp \
shared/03132500.xhp \
shared/05060700.xhp \
shared/keys.xhp \
shared/main0211.xhp \
shared/main0601.xhp \
guide/control_properties.xhp


.IF "$(MAKETARGETS)"!="genPO"
.INCLUDE :	settings.mk
.INCLUDE : $(PRJ)$/settings.pmk

.INCLUDE :  target.mk
.INCLUDE : tg_help.mk
.ELSE
.INCLUDE :  settings.mk
.INCLUDE :  target.mk
.ENDIF
