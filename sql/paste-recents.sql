  select *
    from `paste`
   where `visible` = 1
order by `start` desc
   limit ?
