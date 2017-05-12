require("bbip");
bip = bbip.new("BBIP.dat");
bip:preload()
for i = 1, 10000000 do
	range_start, range_end, country, province, city, district, isp, type, desc, lat, lng, cidr = bip:query("223.5.5.5")
	-- print(range_start, range_end, country, province, city, district, isp, type, desc, lat, lng, cid)
	-- rc = bip:query_table("223.5.5.5")
	-- print(country, province)
end
-- print("OK",query.start, query['end'], query.country, query.province, query.city, query.district, query.isp, query.type, query.desc, query.lat, query.lng)
