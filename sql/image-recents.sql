  select length(`data`)
       , *
    from `image`
   where `visible` = 1
order by `start` desc
   limit ?
