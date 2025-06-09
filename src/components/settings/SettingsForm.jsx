import * as React from "react";
import {
  makeStyles,
  useId,
  Input,
  Label,
  Button,
  MessageBar,
} from "@fluentui/react-components";
import { useState, useEffect } from "react";
import { useAuth } from "../../context/AuthContext";
import { useMutation, useQueryClient } from "@tanstack/react-query";

const useStyles = makeStyles({
  root: {
    // Stack the label above the field
    display: "flex",
    flexDirection: "column",
    // Use 2px gap below the label (per the design system)
    gap: "2px",
    // Prevent the example from taking the full width of the page (optional)
    maxWidth: "400px",
  },
  inputField: {
    margin: "4px",
    marginLeft: "16px",
  },
  label: {
    marginTop: "10px",
    marginLeft: "12px",
  },
});

export const SettingsForm = ({ changingValue }) => {
  const inputId = useId("input");
  const classes = useStyles();
  const [changing, setChanging] = useState(false);
  const { user, token, setUser } = useAuth();
  const [value, setValue] = useState("");
  const queryClient = useQueryClient();
  const [error, setError] = useState();
  const [isPassword, setIsPassword] = useState(false);
  const [oldPassword, setOldPassword] = useState("");

  useEffect(() => {
    console.log("Changing value:", changingValue);
    console.log("User from context:", user);

    switch (changingValue) {
      case "username":
        setValue(user?.username ?? "");
        break;

      case "email":
        setValue(user?.email ?? "");
        break;

      case "password":
        setValue("");
        setIsPassword(true);
        break;

      default:
        break;
    }
  }, [changingValue, user]);

  // Mutation for updating username
  const { mutate: changeUsernameMutation, isLoading: isUpdatingUsername } =
    useMutation({
      mutationFn: (newUsername) =>
        fetch("http://157.230.16.67:8000/api/usernameupdate", {
          method: "PUT",
          headers: {
            "Content-Type": "application/json",
            Authorization: `Bearer ${token}`,
          },
          body: JSON.stringify({
            username: user.username, // current username from context
            new_username: newUsername,
          }),
        }).then((res) => {
          if (!res.ok) {
            throw new Error("Failed to update username");
          }
          return res.json();
        }),
      onSuccess: (data) => {
        console.log("Username updated successfully:", data);
        // Optionally: update user context or refetch data
        queryClient.invalidateQueries(["userData"]);
        setUser({ ...user, username: data.user.username });
        setError();
      },
      onError: (error) => {
        console.error("Error updating username:", error);
        setError(error);
      },
    });

  // Mutation for updating email
  const { mutate: changeEmailMutation, isLoading: isUpdatingEmail } =
    useMutation({
      mutationFn: (newEmail) =>
        fetch("http://157.230.16.67:8000/api/emailupdate", {
          method: "PUT",
          headers: {
            "Content-Type": "application/json",
            Authorization: `Bearer ${token}`,
          },
          body: JSON.stringify({
            email: user.email, // current email from context
            new_email: newEmail,
          }),
        }).then((res) => {
          if (!res.ok) {
            throw new Error("Failed to update email");
          }
          return res.json();
        }),
      onSuccess: (data) => {
        console.log("Email updated successfully:", data);
        queryClient.invalidateQueries(["userData"]);
        setUser({ ...user, email: data.user.email });
      },
      onError: (error) => {
        console.error("Error updating email:", error);
        setError(error);
      },
    });

  // Mutation for updating password
  const { mutate: changePasswordMutation, isLoading: isUpdatingPassword } =
    useMutation({
      mutationFn: (newPassword) =>
        fetch("http://157.230.16.67:8000/api/passwordupdate", {
          method: "PUT",
          headers: {
            "Content-Type": "application/json",
            Authorization: `Bearer ${token}`,
          },
          body: JSON.stringify({
            // Optionally add current password if needed:
            password: oldPassword, // current password (if required)
            new_password: newPassword,
          }),
        }).then((res) => {
          if (!res.ok) {
            throw new Error("Failed to update password");
          }
          return res.json();
        }),
      onSuccess: (data) => {
        console.log("Password updated successfully:", data);
        queryClient.invalidateQueries(["userData"]);
        setError();
        setValue("");
      },
      onError: (error) => {
        console.error("Error updating password:", error);
        setError(error);
      },
    });

  // Combined save handler that calls the appropriate mutation based on changingValue
  const handleSave = () => {
    if (changingValue === "username") {
      changeUsernameMutation(value);
    } else if (changingValue === "email") {
      changeEmailMutation(value);
    } else if (changingValue === "password") {
      changePasswordMutation(value);
    }
    setChanging(false);
  };

  const isUpdating =
    isUpdatingUsername || isUpdatingEmail || isUpdatingPassword;

  return (
    <div className={classes.root}>
      {changing && isPassword && (
        <div style={{ marginTop: "8px" }}>
          <Label htmlFor={oldPassword} className={classes.label}>
            Enter old password:
          </Label>
          <div>
            <Input
              id={oldPassword}
              className={classes.inputField}
              onChange={(e) => setOldPassword(e.target.value)}
            />
          </div>
        </div>
      )}
      <Label htmlFor={inputId} className={classes.label}>
        Change {changingValue}:
      </Label>
      <div>
        <Input
          id={inputId}
          className={classes.inputField}
          value={value}
          onChange={(e) => setValue(e.target.value)}
          disabled={!changing}
        />
        {!changing && <Button onClick={() => setChanging(true)}>Change</Button>}
        {changing && (
          <div style={{ marginLeft: "16px" }}>
            <Button appearance="primary" onClick={handleSave}>
              Save
            </Button>{" "}
            <Button onClick={() => setChanging(false)}>Cancel</Button>
          </div>
        )}
      </div>
      {error && <MessageBar intent="error">{error.message}</MessageBar>}
    </div>
  );
};
