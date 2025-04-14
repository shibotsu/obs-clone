import { useState } from "react";
import { LargeTitle, makeStyles, Divider } from "@fluentui/react-components";
import { SettingsForm } from "../../components/settings/SettingsForm";

const useStyles = makeStyles({
  title: {
    padding: "4px",
  },
  settingsContainer: {
    display: "flex",
    flexDirection: "column",
    padding: "8px",
    margin: "auto",
  },
});

const SettingsPage = () => {
  const classes = useStyles();

  return (
    <div className={classes.settingsContainer}>
      <LargeTitle className={classes.title}>Settings</LargeTitle>
      <Divider />
      <SettingsForm changingValue="username" />
      <SettingsForm changingValue="email" />
      <SettingsForm changingValue="password" />
    </div>
  );
};

export default SettingsPage;
