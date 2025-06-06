import { useState, useRef } from "react";
import {
  Button,
  Input,
  Textarea,
  Dropdown,
  Option,
  Card,
  Text,
  Spinner,
  MessageBar,
  MessageBarBody,
  MessageBarTitle,
  Image,
  Field,
  makeStyles,
  tokens,
  shorthands,
} from "@fluentui/react-components";
import {
  PlayRegular,
  CopyRegular,
  CheckmarkRegular,
  VideoRegular,
  ArrowClockwiseRegular,
  ImageAddRegular,
} from "@fluentui/react-icons";
import { useQuery, useMutation } from "@tanstack/react-query";
import { useAuth } from "../../context/AuthContext";
import { use } from "react";

const useStyles = makeStyles({
  container: {
    ...shorthands.padding("24px"),
    backgroundColor: tokens.colorNeutralBackground2,
    minHeight: "100vh",
    display: "flex",
    flexDirection: "column",
    alignItems: "center",
    ...shorthands.gap("24px"),
  },

  title: {
    fontSize: tokens.fontSizeHero700,
    fontWeight: tokens.fontWeightSemibold,
    color: tokens.colorNeutralForeground1,
    textAlign: "center",
  },

  card: {
    width: "100%",
    maxWidth: "600px",
    ...shorthands.padding("32px"),
  },

  formStack: {
    display: "flex",
    flexDirection: "column",
    ...shorthands.gap("20px"),
  },

  buttonGroup: {
    display: "flex",
    ...shorthands.gap("12px"),
    alignItems: "center",
  },

  streamKeyContainer: {
    display: "flex",
    flexDirection: "column",
    ...shorthands.gap("12px"),
  },

  streamKeyInput: {
    display: "flex",
    ...shorthands.gap("8px"),
    alignItems: "flex-end",
  },

  streamKeyField: {
    flexGrow: 1,
  },

  copyButton: {
    minWidth: "40px",
  },

  instructionText: {
    fontSize: tokens.fontSizeBase200,
    color: tokens.colorNeutralForeground2,
  },

  actionButtons: {
    display: "flex",
    ...shorthands.gap("12px"),
    ...shorthands.margin("16px", "0", "0", "0"),
  },

  messageBar: {
    ...shorthands.margin("0", "0", "16px", "0"),
  },
});

const StartStreamingPage = () => {
  const styles = useStyles();
  const [streamName, setStreamName] = useState("");
  const [description, setDescription] = useState("");
  const [selectedCategory, setSelectedCategory] = useState("");
  const [streamKey, setStreamKey] = useState("");
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState("");
  const [success, setSuccess] = useState(false);
  const [copied, setCopied] = useState(false);
  const [thumbnail, setThumbnail] = useState(null);
  const fileInputRef = useRef(null);
  const { token, user } = useAuth();

  const { data, refetch } = useQuery({
    queryKey: ["channelData"],
    queryFn: async () => {
      const response = await fetch(
        `http://127.0.0.1:8000/api/channel/${user.id}`,
        {
          headers: {
            "Content-Type": "application/json",
            Authorization: `Bearer ${token}`,
          },
        }
      );

      if (!response.ok) {
        return new Error("Unable to fetch channel data.");
      }

      return response.json();
    },
  });

  const channel = data?.channel;

  const startStreamMutation = useMutation({
    mutationKey: ["startStream"],
    mutationFn: async (payload) => {
      const response = await fetch("http://127.0.0.1:8000/api/startstream", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
          Authorization: `Bearer ${token}`,
        },
        body: JSON.stringify(payload),
      });
      if (!response.ok) {
        return new Error("Unable to start stream.");
      }
    },
  });

  const endStreamData = { user_id: user.id };

  const stopStreamMutation = useMutation({
    mutationKey: ["stopStream"],
    mutationFn: async () => {
      console.log("Calling stop stream API");
      const response = await fetch("http://127.0.0.1:8000/api/endstream", {
        method: "PUT",
        headers: {
          "Content-Type": "application/json",
          Authorization: `Bearer ${token}`,
        },
        body: JSON.stringify(endStreamData),
      });
    },
    onSuccess: () => refetch(),
  });

  // Sample categories - replace with your actual categories
  const categories = [
    "Gaming",
    "Music",
    "Art & Creative",
    "Education",
    "Technology",
    "Cooking",
    "Fitness & Health",
    "Travel",
    "Talk Shows",
    "Other",
  ];

  const handleStartStream = async () => {
    if (!streamName.trim()) {
      setError("Stream name is required");
      return;
    }

    if (!selectedCategory) {
      setError("Please select a category");
      return;
    }

    setIsLoading(true);
    setError("");

    const payload = {
      stream_title: streamName,
      stream_description: description,
      stream_category: selectedCategory,
      stream_key: channel?.stream_key,
      thumbnail: thumbnail,
    };

    console.log(channel.stream_key);

    await startStreamMutation.mutateAsync(payload);

    setStreamKey(channel?.stream_key);
    setSuccess(true);
  };

  const handleCopyStreamKey = async () => {
    try {
      await navigator.clipboard.writeText(streamKey);
      setCopied(true);
      setTimeout(() => setCopied(false), 2000);
    } catch (err) {
      setError("Failed to copy stream key");
    }
  };

  const handleReset = () => {
    setStreamName("");
    setDescription("");
    setSelectedCategory("");
    setStreamKey("");
    setSuccess(false);
    setError("");
    setIsLoading(false);
    setCopied(false);
  };

  const handleThumbnailClick = () => {
    fileInputRef.current?.click();
  };

  const handleThumbnailChange = (e) => {
    if (e.target.files && e.target.files[0]) {
      setThumbnail(e.target.files[0]);
    }
  };

  return (
    <div className={styles.container}>
      <Text className={styles.title}>Start Your Stream</Text>

      <Card className={styles.card}>
        <div className={styles.formStack}>
          {error && (
            <MessageBar intent="error" className={styles.messageBar}>
              <MessageBarBody>
                <MessageBarTitle>Error</MessageBarTitle>
                {error}
              </MessageBarBody>
            </MessageBar>
          )}

          {success && !error && (
            <MessageBar intent="success" className={styles.messageBar}>
              <MessageBarBody>
                <MessageBarTitle>Success</MessageBarTitle>
                Stream setup completed! Your stream key is ready.
              </MessageBarBody>
            </MessageBar>
          )}

          <Field label="Stream Name" required>
            <Input
              placeholder="Enter your stream title"
              value={streamName}
              onChange={(e) => setStreamName(e.target.value)}
              disabled={success}
            />
          </Field>

          <Field label="Description (Optional)">
            <Textarea
              placeholder="Describe what you'll be streaming about..."
              rows={3}
              value={description}
              onChange={(e) => setDescription(e.target.value)}
              disabled={success}
            />
          </Field>

          <Field label="Category" required>
            <Dropdown
              placeholder="Select a category"
              value={selectedCategory}
              selectedOptions={selectedCategory ? [selectedCategory] : []}
              onOptionSelect={(e, data) =>
                setSelectedCategory(data.optionValue || "")
              }
              disabled={success}
            >
              {categories.map((category) => (
                <Option key={category} value={category}>
                  {category}
                </Option>
              ))}
            </Dropdown>
          </Field>

          <Text weight="semibold">Upload Stream Thumbnail</Text>

          <input
            type="file"
            accept="image/*"
            onChange={handleThumbnailChange}
            ref={fileInputRef}
            style={{ display: "none" }}
          />

          <Button
            icon={<ImageAddRegular />}
            onClick={handleThumbnailClick}
            appearance="secondary"
          >
            {thumbnail ? "Change Thumbnail" : "Upload Thumbnail"}
          </Button>

          {thumbnail && (
            <Image
              src={URL.createObjectURL(thumbnail)}
              alt="Thumbnail preview"
              fit="cover"
              style={{
                marginTop: 12,
                maxWidth: "100%",
                borderRadius: 8,
                maxHeight: 200,
              }}
            />
          )}

          {!success && (
            <div className={styles.buttonGroup}>
              {channel?.is_live === 0 ? (
                <Button
                  appearance="primary"
                  icon={isLoading ? <Spinner size="tiny" /> : <PlayRegular />}
                  onClick={handleStartStream}
                  disabled={isLoading}
                >
                  {isLoading ? "Generating..." : "Start Stream"}
                </Button>
              ) : (
                <Button onClick={() => stopStreamMutation.mutateAsync()}>
                  End Stream
                </Button>
              )}
            </div>
          )}

          {success && streamKey && (
            <div className={styles.streamKeyContainer}>
              <Text weight="semibold" size={400}>
                Your Stream Key
              </Text>

              <div className={styles.streamKeyInput}>
                <Field className={styles.streamKeyField}>
                  <Input
                    value={streamKey}
                    readOnly
                    style={{
                      fontFamily: "monospace",
                      backgroundColor: tokens.colorNeutralBackground3,
                    }}
                  />
                </Field>
                <Button
                  appearance="primary"
                  icon={copied ? <CheckmarkRegular /> : <CopyRegular />}
                  onClick={handleCopyStreamKey}
                  className={styles.copyButton}
                  style={{
                    backgroundColor: copied
                      ? tokens.colorPaletteGreenBackground3
                      : undefined,
                  }}
                />
              </div>

              <Text className={styles.instructionText}>
                Copy this key and paste it into your streaming software (OBS,
                XSplit, etc.)
              </Text>

              <div className={styles.actionButtons}>
                <Button appearance="primary" icon={<VideoRegular />}>
                  Go to Stream Dashboard
                </Button>
                <Button
                  appearance="secondary"
                  icon={<ArrowClockwiseRegular />}
                  onClick={handleReset}
                >
                  Start New Stream
                </Button>
              </div>
            </div>
          )}
        </div>
      </Card>
    </div>
  );
};

export default StartStreamingPage;
