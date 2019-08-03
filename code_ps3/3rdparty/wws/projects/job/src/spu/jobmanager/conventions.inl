/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

.defcc StoreAuditDataInitialNoDataU64CC [linkr($78), vol($74-$77), in($74)]
.defcc StoreAuditDataInitialNoParmsU64CC [linkr($78), vol($74-$77), in($74-$75)]
.defcc StoreAuditDataInitialU64CC [linkr($78), vol($74-$77), in($74-$76)]
.defcc StoreAuditDataParameterU64CC [linkr($78), vol($74-$77), in($74)]
.defcc OutputAuditBlockCC [linkr($78), vol($74-$77)]
.defcc IsDmaTagMaskDoneCC [linkr($4), vol($3,$74,$75), in($3), out($3)]
.defcc UseDmaTagIdCC [linkr($4), vol($3, $5, $74-$78), out($3)]
.defcc FreeDmaTagIdCC [linkr($4), vol($74-$78), in($3)]
.defcc GetPageMasksCC [linkr($5), vol($3,$4,$73-$79), in($3-$5), out($3-$4)]
.defcc GetLogicalBufferCC [linkr($6), vol($20-$32, $60-$77), in($3-$6), out($20-$32)]
.defcc TryFreeTagAndUsedPagesCC [linkr($6), vol($3-$5, $69-$79), in($3-$5), out($3)]
.defcc TryDumpShareBufCC [linkr($8), vol($3-$5, $57-$59, $73, $74-$79), in($3-$7), out($3)]
.defcc SetMemCC [linkr($6), vol($70-$79), in($3-$5)]
.defcc TryDumpAllStoreShareBufsCC [linkr($4), vol($3-$8, $57-$59, $64-$69, $73, $74-$79), out($3)]
.defcc StartBarrieredNullListWithInterruptCC [linkr($4), vol($3-$5, $77), in($3)]
.defcc AllocateJobCC [linkr($5), vol($3-$4, $50-$73), in($3-$4), out($3)]
.defcc ChangeLoadToRunJobCC [linkr($5), in($lr), vol($0, $2-$79)]
.defcc StoreAuditCC [linkr($78), vol($74-$77), in($74-$76)]
