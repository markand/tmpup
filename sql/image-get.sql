select length(`data`)
     , *
  from `image`
 where `id` = ?
 limit 1
