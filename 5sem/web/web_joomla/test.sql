SELECT c.title, u.name, c.level
FROM #__categories AS c
INNER JOIN #__users AS u
	ON u.id = c.created_user_id
ORDER BY u.name