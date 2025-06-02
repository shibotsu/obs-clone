import React, { useState } from "react";
import { Button, Tooltip } from "@fluentui/react-components";
import { Share16Regular } from "@fluentui/react-icons";

export default function ShareButton() {
  const [copied, setCopied] = useState(false);

  const handleShare = async () => {
    try {
      await navigator.clipboard.writeText(window.location.href);
      setCopied(true);
      setTimeout(() => setCopied(false), 2000); // reset after 2 seconds
    } catch (err) {
      console.error("Failed to copy:", err);
    }
  };

  return (
    <Tooltip
      content={copied ? "Link copied!" : "Copy shareable link"}
      relationship="label"
      positioning="above"
    >
      <Button
        icon={<Share16Regular />}
        onClick={handleShare}
        appearance="subtle"
      />
    </Tooltip>
  );
}
