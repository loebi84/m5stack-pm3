#!/bin/sh
set -e
: ${GH_USER:?set GH_USER}; : ${GH_REPO:?set GH_REPO}; : ${GH_TOKEN:?set GH_TOKEN}
ART_URL=$(curl -s -H "Authorization: token $GH_TOKEN" https://api.github.com/repos/$GH_USER/$GH_REPO/actions/artifacts | jq -r '.artifacts|sort_by(.created_at)|last|.archive_download_url')
[ "$ART_URL" = "null" ] && { echo "No artifacts found"; exit 1; }
curl -L -H "Authorization: token $GH_TOKEN" -o firmware.zip "$ART_URL"
unzip -o firmware.zip >/dev/null
echo "firmware.bin ready."
if [ -n "$M5_IP" ]; then
  curl -F "firmware=@firmware.bin" http://$M5_IP/update
fi
